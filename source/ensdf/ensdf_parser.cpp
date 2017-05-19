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




DaughterParser::DaughterParser()
{

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
  //  Q_ASSERT(decays_.size() == decays_.keys().toSet().size()); // check for duplicates
  std::list<NuclideId> ret;
  for (auto &n : nuclide_data_)
    ret.push_back(n.first);
  return ret;
}

std::list<std::string> DaughterParser::decays(NuclideId daughter) const
{
  //  Q_ASSERT(decays_.value(daughterNuclide).size() == decays_.value(daughterNuclide).keys().toSet().size()); // check for duplicates
  std::list<std::string> result;
  for (auto &i : nuclide_data_.at(daughter).decays)
    result.push_back(i.first);
  return result;
}

void DaughterParser::LevelIndex::find(BlockIndices alpos,
                                      std::string dNucid1, std::string dsid,
                                      const std::vector<std::string>& data)
{
  int laststart = -1;
  for (size_t i=alpos.first; i < alpos.last; i++)
  {
    const std::string line = data.at(i);
    // extract cross reference records as long as first level was found (cross reference must be before 1st level)
    if (laststart == -1)
    {
      if (line.substr(0,8) == (dNucid1 + "  X"))
        xrefs[boost::trim_copy(line.substr(9, 30))] = line[8];
    }
    // find level records
    if (line.substr(0,9) == (dNucid1 + "  L "))
    {
      if (laststart > 0)
        insertAdoptedLevelsBlock(BlockIndices(laststart, i),
                                 dsid, data);
      laststart = i;
    }
  }
  if (laststart > 0)
    insertAdoptedLevelsBlock(BlockIndices(laststart, alpos.last),
                             dsid, data);
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
  auto xreflist = extractContinuationRecords(newblock, {"XREF"}, data);
  std::string xref = xreflist.at(0);

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


DecayScheme DaughterParser::get_decay(NuclideId daughter,
                                      std::string decay_name) const
{
  if (!nuclide_data_.count(daughter) ||
      !nuclide_data_.at(daughter).decays.count(decay_name))
    return DecayScheme();

  double adoptedLevelMaxDifference = 40.0;
  double gammaMaxDifference = 5.0;

  BlockIndices alpos = nuclide_data_.at(daughter).adopted_levels.block;
  DecayData decaydata = nuclide_data_.at(daughter).decays.at(decay_name);

  Nuclide daughter_nuclide(decaydata.id.nuclide);

  double normalizeDecIntensToPercentParentDecay = 1.0;
  double normalizeGammaIntensToPercentParentDecay = 1.0;
  std::string dNucid1 = nid_to_ensdf(daughter, true);
  std::string dNucid2 = nid_to_ensdf(daughter, false);

  //  DBG << "parsing " << dNucid.toStdString();

  LevelIndex levels_index_;
  levels_index_.find(alpos, dNucid1, decaydata.id.dsid, raw_contents_);
  levels_index_.find(alpos, dNucid2, decaydata.id.dsid, raw_contents_);

  // process all adopted level sub-blocks
  Level currentLevel;
  // adopted levels block of current level in decay data set
  BlockIndices currentadoptblock;
  for (size_t k=decaydata.block.first; k < decaydata.block.last; k++)
  {
    const std::string line = raw_contents_.at(k);

    //    DBG << "c line " << line.toStdString();

    // process new gamma
    if (is_gamma_line(line, dNucid1, dNucid2))
    {
      Energy energy = Energy(parse_val_uncert(line.substr(9, 10),
                                              line.substr(19, 2)));
      std::string multipolarity = boost::trim_copy(line.substr(31, 10));
      UncertainDouble delta = parseEnsdfMixing(line.substr(41, 14), multipolarity);

      double intensity = norm(line.substr(21,8),
                              std::numeric_limits<double>::quiet_NaN());
      if (std::isfinite(intensity))
        intensity *= normalizeGammaIntensToPercentParentDecay;

      // parse adopted levels if necessary
      if ((delta.sign() != UncertainDouble::SignMagnitudeDefined) ||
          multipolarity.empty())
      {
        auto e2g =
            get_gamma_lines(raw_contents_, currentadoptblock, daughter);
        // find gamma
        if (!e2g.empty())
        {
          Energy foundE = findNearest(e2g, energy);
          if (energy-foundE < gammaMaxDifference/1000.0*energy)
          {
            const std::string gammastr = e2g.at(foundE);
            if (multipolarity.empty())
              multipolarity = boost::trim_copy(gammastr.substr(31, 10));

            if (delta.sign() != UncertainDouble::SignMagnitudeDefined)
            {
              UncertainDouble adptdelta = parseEnsdfMixing(gammastr.substr(41, 14), multipolarity);
              if (adptdelta.sign() > delta.sign())
                delta = adptdelta;
            }
          }
        }
      }

      // determine levels
      Energy from = currentLevel.energy();
      Energy to = findNearest(daughter_nuclide.levels(), from - energy);
      daughter_nuclide.addTransition(Transition(energy, intensity,
                                                multipolarity, delta,
                                                from, to));
    }
    // process new level
    else if (is_level_line(line, dNucid1, dNucid2))
    {

      currentLevel = parse_level(line);

      // get additional data from adopted leves record
      //   find closest entry
      currentadoptblock.clear();
      Energy foundE = findNearest(levels_index_.adoptblocks,
                                  currentLevel.energy());
      if (std::abs((currentLevel.energy() - foundE).operator double()) <=
          (adoptedLevelMaxDifference/1000.0*currentLevel.energy()))
        currentadoptblock = levels_index_.adoptblocks.at(foundE);

      // if an appropriate entry was found, read its raw_contents_
      // set half life if necessary
      if (currentadoptblock.first != currentadoptblock.last)
      {
        std::string levelfirstline(raw_contents_.at(currentadoptblock.first));
        //                DBG << levelfirstline << " === " << currentLevel->to_string();
        if (!currentLevel.halfLife().isValid())
        {
          currentLevel.set_halflife(parse_halflife(levelfirstline.substr(39, 16)));
          if (!currentLevel.spin().valid())
            currentLevel.set_spin(parse_spin_parity(levelfirstline.substr(21, 18)));
          //          DBG << "newhl" << currentLevel->halfLife().to_string();
        }
        // parse continuation records
        auto moms = extractContinuationRecords(currentadoptblock,
        {"MOME2", "MOMM1"},
                                               raw_contents_);
        currentLevel.set_q(parse_moment(moms.at(0)));
        currentLevel.set_mu(parse_moment(moms.at(1)));
        //                DBG << levelfirstline << " === " << currentLevel->to_string();
      }

      daughter_nuclide.addLevel(currentLevel);
    }
    // process decay information
    else if (!daughter_nuclide.empty() &&
             (is_intensity_line(line, dNucid1, dNucid2)))
    {
      UncertainDouble ti = parse_val_uncert(line.substr(64, 10), line.substr(74, 2));
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType)
        ti.setSign(UncertainDouble::SignMagnitudeDefined);
      else
      {
        UncertainDouble ib = parse_val_uncert(line.substr(21, 8), line.substr(29, 2));
        UncertainDouble ie = parse_val_uncert(line.substr(31, 8), line.substr(39, 2));
        ti = ib;
        if (ib.uncertaintyType() != UncertainDouble::UndefinedType && ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti += ie;
        else if (ie.uncertaintyType() != UncertainDouble::UndefinedType)
          ti = ie;
        else
          ti = UncertainDouble();
      }
      if (ti.uncertaintyType() != UncertainDouble::UndefinedType)
      {
        currentLevel.setFeedIntensity(ti);
        daughter_nuclide.addLevel(currentLevel);
      }
    }
    else if (!daughter_nuclide.empty() &&
             (is_feed_line(line, dNucid1, dNucid2)))
    {
      UncertainDouble ib = parse_val_uncert(line.substr(21, 8), line.substr(29, 2));
      if (ib.hasFiniteValue())
      {
        ib.setSign(UncertainDouble::SignMagnitudeDefined);
        ib *= normalizeDecIntensToPercentParentDecay;
        currentLevel.setFeedIntensity(ib);
        daughter_nuclide.addLevel(currentLevel);
      }
    }
    // process normalization records
    else if (is_norm_line(line, dNucid1, dNucid2))
    {
      double br = norm(line.substr(31, 8), 1.0);
      double nb = norm(line.substr(41, 8), 1.0);
      normalizeDecIntensToPercentParentDecay = nb * br;
      double nr = norm(line.substr(9, 10), 1.0);
      normalizeGammaIntensToPercentParentDecay = nr * br;
    }
    else if (is_p_norm_line(line, dNucid1, dNucid2))
    {
      normalizeDecIntensToPercentParentDecay =
          norm(line.substr(41, 8), normalizeDecIntensToPercentParentDecay);
      normalizeGammaIntensToPercentParentDecay =
          norm(line.substr(9, 10), normalizeGammaIntensToPercentParentDecay);
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

double DaughterParser::norm(std::string rec, double def_value)
{
  boost::replace_all(rec, "(", "");
  boost::replace_all(rec, ")", "");
  boost::trim(rec);
  if (is_number(rec))
    return boost::lexical_cast<double>(rec);
  return def_value;
}

UncertainDouble DaughterParser::parseEnsdfMixing(const std::string &mstr,
                                                 const std::string &multipolarity)
{
  if (mstr.size() != 14)
    return UncertainDouble();

  // special case for pure multipolarities
  if (boost::trim_copy(mstr).empty())
  {
    std::string tmp(multipolarity);
    boost::replace_all(tmp, "(", "");
    boost::replace_all(tmp, ")", "");
    if (tmp.size() == 2)
      return UncertainDouble(0.0, 1, UncertainDouble::SignMagnitudeDefined);
    else
      return UncertainDouble();
  }

  return parse_val_uncert(mstr.substr(0,8), mstr.substr(8,6));
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

void DaughterParser::parse_comments_block(BlockIndices block_idx)
{
  for (size_t i = block_idx.first + 1; i < block_idx.last; ++i)
  {
    if (HistoryRecord::match(raw_contents_[i]))
      mass_history_.push_back(HistoryRecord(i, raw_contents_));
    else if (CommentsRecord::match(raw_contents_[i], "\\s"))
      mass_comments_.push_back(CommentsRecord(i, raw_contents_));
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

    if (test(header.type & RecordType::Comments) &&
        !header.nuclide.composition_known())
    {
      parse_comments_block(block_idx);
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
      if (nuclide_data_.count(lev.id.nuclide))
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

      if (!nuclide_data_.count(decaydata.id.nuclide))
        DBG << "No index for " << decaydata.id.nuclide.symbolicName()
            << " exists";
      else
        nuclide_data_[decaydata.id.nuclide].add_decay(decaydata);

      if (test(header.type & RecordType::Decay) ||
          (test(header.type & RecordType::InelasticScattering)))
      {
        if (!decaydata.decay_info_.mode.valid())
        {
          DBG << " BAD DECAY " << decaydata.to_string();
          continue;
        }

        // get reference to daughter map to work with
        // (create and insert if necessary)
//        std::map<std::string, DecayData> &decmap =
//            decays_[decaydata.id.nuclide];

//        auto decayname = decaydata.name();

//        // insert into decay map
//        while (decmap.count(decayname))
//          decayname += " (alt.)";
//        decmap[decayname] = decaydata;

        //      DBG << "Got decay " << header.debug() << " as " << decayname;
      }
    }
    else
    {
      DBG << "ID type bad " << header.extended_dsid;
    }
  }

}
