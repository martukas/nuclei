#include "ensdf_parser.h"
#include "ensdf_types.h"

#include "alpha_record.h"
#include "beta_record.h"
#include "ec_record.h"
#include "gamma_record.h"
#include "history_record.h"
#include "level_record.h"
#include "normalization_record.h"
#include "particle_record.h"
#include "prod_normalization_record.h"
#include "qvalue_record.h"
#include "reference_record.h"
#include "xref_record.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>


ENSDFParser::ENSDFParser()
{}

ENSDFParser::ENSDFParser(std::string directory)
{
  if (directory.empty())
    return;

  boost::system::error_code c;
  boost::filesystem::path path(directory);
  bool is_dir = boost::filesystem::is_directory(path, c);

  if (!is_dir)
    return;

  directory_ = directory;

  boost::filesystem::directory_iterator end_iter;

  for( boost::filesystem::directory_iterator dir_iter(path);
       dir_iter != end_iter ; ++dir_iter)
    if (boost::filesystem::is_regular_file(dir_iter->status()) &&
        (dir_iter->path().string().size() > 3))
    {
      std::string filename = dir_iter->path().string();
      std::string a_num = filename.substr(filename.size()-3, 3);
      if (is_number(a_num))
        masses_.insert(boost::lexical_cast<uint16_t>(a_num));
    }
}

bool ENSDFParser::good() const
{
  return !masses_.empty();
}

std::list<uint16_t> ENSDFParser::masses() const
{
  return std::list<uint16_t>(masses_.begin(), masses_.end());
}

std::string ENSDFParser::directory() const
{
  return directory_;
}

DaughterParser ENSDFParser::get_dp(uint16_t a)
{
  if (!masses_.count(a))
    return DaughterParser();
  else if (!cache_.count(a))
    cache_[a] = DaughterParser(a, directory_);
  return cache_[a];
}



DaughterParser::DaughterParser(uint16_t A, std::string directory)
{
  std::string num = std::to_string(A);
  if (num.size() < 3)
    num = std::string(3-num.size(), '0') + num;
  std::string file = directory + "/ensdf." + num;

  std::ifstream t(file);
  std::string str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)),
             std::istreambuf_iterator<char>());

  boost::split(raw_contents_, str, boost::is_any_of("\n"));

  parseBlocks();
}

std::list<NuclideId> DaughterParser::daughters() const
{
  std::list<NuclideId> ret;
  for (auto &n : nuclide_data_)
    ret.push_back(n.first);
  return ret;
}

std::list<std::string> DaughterParser::decays(NuclideId daughter) const
{
  std::list<std::string> result;
  for (auto &i : nuclide_data_.at(daughter).decays)
    result.push_back(i.first);
  return result;
}

/**
 * @brief DaughterParser::insertAdoptedLevelsBlock
 * @param adoptblocks map of adopted levels (target)
 * @param newblock block to add
 * @param dssym Symbol of the current decay. Used to filter levels and adjust energies according to XREF records
 */
void DaughterParser::LevelIndex::insertAdoptedLevelsBlock(const BlockIndices &newblock,
                                                          const std::string& dsid,
                                                          const std::vector<std::string> &data)
{
  if ((newblock.first >= newblock.last) ||
      (newblock.last > data.size()) ||
      !xrefs.count(dsid))
    return;

  // get xref record
//  auto xreflist = extractContinuationRecords(newblock, {"XREF"}, data);
  std::string xref;// = xreflist.at(0);

  // filter data sets

  // -(AB) case (do not add level if dssym is contained in the parentheses)
  if ((xref.substr(0,2) == "-(")
      && (xref[xref.size()-1] == ')')
      && boost::contains(xref, dsid))
    return;

  // exit if xref is neither "+" (level valid for all datasets) nor -(...) not containing dssymb
  // nor contains dssym
  if (xref != "+"
      && (xref.substr(0,2) != "-(")
      && !boost::contains(xref, dsid))
    return;

  // if this point is reached the level will be added in any case

  // read energy from level record
  auto edata = data.at(newblock.first);
  Energy e = Energy(parse_val_uncert(edata.substr(9, 10), edata.substr(19, 2)));

  // for the A(E1) case the energy must be modified
  boost::smatch what;
  if (boost::regex_match(xref, what, boost::regex(dsid + "\\(([.\\d]+)\\)")) &&
      (what.size() > 1))
  {
    std::string edata(what[1]);
    std::string val, uncert;
    if (edata.size() > 10)
    {
      val = edata.substr(0,10);
      uncert = edata.substr(10,2);
    }
    else
      val = edata;
    Energy matchedE = Energy(parse_val_uncert(val, uncert));
    DBG << "ENERGY FROM REGEXP " << xref << " --> "
        << matchedE.to_string() << " valid "
        << matchedE.valid();
    //std::cerr << "Current xref: " << xref.toStdString() << " current dssymb: " << dssym << std::endl;
    //std::cerr << "Energy translation, old: " << e << " new: " << matchedE << std::endl;
    if (matchedE.valid())
      e = matchedE;
  }

  // add record
  adoptblocks[e] = newblock;
}

void DaughterParser::modify_delta_pol(const std::list<LevelRecord> &levels,
                                      Energy energy,
                                      std::string& multipolarity,
                                      UncertainDouble& delta,
                                      double maxdif) const
{
  for (const LevelRecord& ll : levels)
  {
    for (GammaRecord& gg : ll.find_nearest(energy))
    {
      if (energy-gg.energy < maxdif/1000.0*energy)
      {
//              DBG << "-1Changing " << multipolarity << " " << delta.to_string(true);

        if (multipolarity.empty())
          multipolarity = gg.multipolarity;

        if (delta.sign() != UncertainDouble::SignMagnitudeDefined)
        {
          UncertainDouble adptdelta = eval_mixing_ratio(gg.mixing_ratio, multipolarity);
          if (adptdelta.sign() > delta.sign())
            delta = adptdelta;
        }
//              DBG << "+1Changing " << multipolarity << " " << delta.to_string(true);
      }
    }
  }
}

void DaughterParser::modify_level(const std::list<LevelRecord>& in,
                                  std::list<LevelRecord>& out,
                                  Level& currentLevel,
                                  double maxdif) const
{
  out.clear();
  for (const LevelRecord& f : in)
  {
    if (std::abs((currentLevel.energy() - f.energy).operator double())
        <= (maxdif/1000.0*currentLevel.energy()))
    {
//          DBG << "Matching E " << currentLevel.energy().to_string();
//          DBG << "FoundE = " << f.energy.to_string();

      out.push_back(f);

      if (!currentLevel.halfLife().isValid())
      {
//            DBG << "- modifying level " << currentLevel.halfLife().to_string(true)
//                << " " << currentLevel.spin().to_string();

        currentLevel.set_halflife(f.halflife);
        if (!currentLevel.spin().valid())
          currentLevel.set_spin(f.spin_parity);

//            DBG << "+ modified level " << currentLevel.halfLife().to_string(true)
//                << " " << currentLevel.spin().to_string();
      }
      // parse continuation records

      if (f.continuations_.count("MOME2"))
      {
        auto mome2 = f.continuations_.at("MOME2");
//            DBG << "MOME2 from continuation " << parse_moment(mome2).to_string();
        currentLevel.set_q(parse_moment(mome2));
      }

      if (f.continuations_.count("MOMM1"))
      {
        auto momm1 = f.continuations_.at("MOMM1");
//            DBG << "MOMM1 from continuation " << parse_moment(momm1).to_string();
        currentLevel.set_mu(parse_moment(momm1));
      }
    }
  }
}



DecayScheme DaughterParser::get_decay(NuclideId daughter,
                                      std::string decay_name) const
{
  if (!nuclide_data_.count(daughter) ||
      !nuclide_data_.at(daughter).decays.count(decay_name))
    return DecayScheme();

  double adoptedLevelMaxDifference = 40.0;
  double gammaMaxDifference = 5.0;

  LevelData leveldata = nuclide_data_.at(daughter).adopted_levels;
  DecayData decaydata = nuclide_data_.at(daughter).decays.at(decay_name);

  Nuclide daughter_nuclide(decaydata.id.nuclide);

  UncertainDouble normalizeDecIntensToPercentParentDecay(1.0,1,UncertainDouble::SignMagnitudeDefined);
  UncertainDouble normalizeGammaIntensToPercentParentDecay(1.0,1,UncertainDouble::SignMagnitudeDefined);

  //multiple times???
  for (auto n : decaydata.norm)
  {
    normalizeDecIntensToPercentParentDecay = n.NB * n.BR;
    normalizeGammaIntensToPercentParentDecay = n.NR * n.BR;
  }

  // process normalization records
  if (decaydata.pnorm.valid())
  {
    if (decaydata.pnorm.NBBR.hasFiniteValue())
      normalizeDecIntensToPercentParentDecay = decaydata.pnorm.NBBR;
    if (decaydata.pnorm.NRBR.hasFiniteValue())
      normalizeGammaIntensToPercentParentDecay = decaydata.pnorm.NRBR;
  }

  for (const LevelRecord& lev : decaydata.levels)
  {
    std::list<LevelRecord> irrelevant_levels;
    Level currentLevel(lev.energy, lev.spin_parity, lev.halflife, lev.isomeric);
    modify_level(leveldata.find_nearest(lev.energy),
                 irrelevant_levels, currentLevel,
                 adoptedLevelMaxDifference);
    for (const AlphaRecord& a : lev.alphas)
    {
      if (a.intensity_alpha.hasFiniteValue())
        currentLevel.setFeedIntensity(a.intensity_alpha * normalizeDecIntensToPercentParentDecay);
    }
    for (const BetaRecord& b : lev.betas)
    {
      if (b.intensity.hasFiniteValue())
        currentLevel.setFeedIntensity(b.intensity * normalizeDecIntensToPercentParentDecay);
    }
    for (const ECRecord& e : lev.ECs)
    {
      if (e.intensity_total.defined())
        currentLevel.setFeedIntensity(e.intensity_total);
      else
        currentLevel.setFeedIntensity(e.intensity_beta_plus + e.intensity_ec);
    }
    daughter_nuclide.addLevel(currentLevel);
  }

  for (const LevelRecord& lev : decaydata.levels)
  {
    std::list<LevelRecord> irrelevant_levels;
    Level currentLevel(lev.energy, lev.spin_parity, lev.halflife, lev.isomeric);
    modify_level(leveldata.find_nearest(lev.energy),
                 irrelevant_levels, currentLevel,
                 adoptedLevelMaxDifference);

    for (const GammaRecord& g : lev.gammas)
    {
      std::string multipolarity = g.multipolarity;
      UncertainDouble delta = g.mixing_ratio;

      // parse adopted levels if necessary
      if ((delta.sign() != UncertainDouble::SignMagnitudeDefined)
          || multipolarity.empty())
      {
        modify_delta_pol(irrelevant_levels, g.energy,
                         multipolarity, delta,
                         gammaMaxDifference);
      }

      // determine levels
      Energy from = currentLevel.energy();
      Energy to;
      for (auto l : leveldata.find_nearest(from - g.energy))
        if (l.valid())
          to = l.energy;

      daughter_nuclide.addTransition(Transition(g.energy,
                                                g.intensity_rel_photons * normalizeGammaIntensToPercentParentDecay,
                                                multipolarity, delta,
                                                from, to));
    }
  }

  // create relevant parent levels and collect parent half-lifes
  Nuclide parent_nuclide;
  if (!decaydata.parents.empty())
  {
    parent_nuclide = Nuclide(decaydata.parents.at(0).nuclide);
    for (ParentRecord p : decaydata.parents)
    {
      parent_nuclide.addHalfLife(p.hl);

      Level plv(p.energy, p.spin, p.hl);
      plv.setFeedingLevel(true);
      parent_nuclide.addLevel(plv);
    }

    if (!parent_nuclide.empty() &&
        (parent_nuclide.levels().begin()->second.energy() > 0.0))
    {
      Level plv(Energy(0.0, UncertainDouble::SignMagnitudeDefined), SpinParity(), HalfLife());
      plv.setFeedingLevel(false);
      parent_nuclide.addLevel(plv);
    }
  }

  parent_nuclide.finalize();
  daughter_nuclide.finalize();
  //HACK types
  return DecayScheme(decay_name, parent_nuclide,
                     daughter_nuclide, decaydata.decay_info_.mode);
}

std::list<BlockIndices> DaughterParser::find_blocks() const
{
  // create list of block boundaries
  // end index points behind last line of block!
  std::list<BlockIndices> blocks;
  size_t from = 0;
  boost::regex emptyline("^\\s*$");
  for (size_t i=0; i < raw_contents_.size(); ++i)
    if (boost::regex_match(raw_contents_.at(i), emptyline))
    {
      if (i-from > 1)
        blocks.push_back(BlockIndices(from, i));
      from = i + 1;
    }
  if (from < raw_contents_.size())
    blocks.push_back(BlockIndices(from, raw_contents_.size()));
  return blocks;
}

void DaughterParser::parse_comments_block(BlockIndices block_idx,
                                          std::list<HistoryRecord> &hist,
                                          std::list<CommentsRecord> &comm)
{
  for (size_t i = block_idx.first + 1; i < block_idx.last; ++i)
  {
    if (HistoryRecord::match(raw_contents_[i]))
      hist.push_back(HistoryRecord(i, raw_contents_));
    else if (CommentsRecord::match(raw_contents_[i], "\\s"))
      comm.push_back(CommentsRecord(i, raw_contents_));
    else
    {
      DBG << "Unidentified record " << raw_contents_[i];
    }
  }
  //      DBG << "PARSED COMMENTS RECORD";
  //      for (auto c : mass_history_)
  //        DBG << c.debug();
  //      for (auto c : mass_comments_)
  //        DBG << c.debug();
}

void DaughterParser::parse_reference_block(BlockIndices block_idx)
{
  for (size_t i = block_idx.first + 1; i < block_idx.last; ++i)
  {
    if (ReferenceRecord::match(raw_contents_[i]))
    {
      auto ref = ReferenceRecord(i, raw_contents_);
      if (ref.valid())
        references_[ref.keynum] = ref.reference;
      else
        DBG << "Invalid reference " << ref.debug()
            << " from " << raw_contents_[i];
    }
    else
    {
      DBG << "Unidentified record " << raw_contents_[i];
    }
  }
  //  DBG << "PARSED REFERENCES RECORD";
  //  for (auto c : references_)
  //    DBG << "  " << c.first << " = " << c.second;
}


void DaughterParser::parseBlocks()
{
  std::list<BlockIndices> blocks = find_blocks();

  for (BlockIndices &block_idx : blocks)
  {
    auto idx = block_idx.first;
    auto header = IdRecord(idx, raw_contents_);

    if (!header.reflect_parse())
      DBG << "Bad header  ==  " << raw_contents_[idx];
    //    DBG << "IdRecord: " << header.extended_dsid;

    if (test(header.type & RecordType::Comments))
    {
      if (header.nuclide.composition_known())
        parse_comments_block(block_idx,
                             nuclide_data_[header.nuclide].history,
                             nuclide_data_[header.nuclide].comments);
      else
        parse_comments_block(block_idx,
                             mass_history_,
                             mass_comments_);
    }
    else if (test(header.type & RecordType::References) &&
        !header.nuclide.composition_known())
    {
      parse_reference_block(block_idx);
    }
    else if (test(header.type & RecordType::AdoptedLevels))
    {
//      adopted_levels_[header.nuclide] = block_idx;
      LevelData lev(raw_contents_, block_idx);
      if (nuclide_data_[lev.id.nuclide].adopted_levels.valid())
        DBG << "Adopted levels for " << lev.id.nuclide.symbolicName()
            << " already exists";
      else
        nuclide_data_[lev.id.nuclide].adopted_levels = lev;
      //      DBG << "Got level " << lev.id.debug();
    }
    else if (header.type != RecordType::Invalid)
    {
      //      DBG << "Getting decay " << header.debug();
      DecayData decaydata(raw_contents_, block_idx);

      if (!decaydata.decay_info_.valid())
        continue;

      if (!nuclide_data_.count(decaydata.id.nuclide))
        DBG << "No index for " << decaydata.id.nuclide.symbolicName()
            << " exists";
      else
      {
        auto name = nuclide_data_[decaydata.id.nuclide].add_decay(decaydata);
        get_decay(decaydata.id.nuclide, name);
      }
    }
    else
    {
      DBG << "ID type bad " << header.extended_dsid;
    }
  }

}
