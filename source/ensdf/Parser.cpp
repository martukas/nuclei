#include "Parser.h"
#include "Reference.h"
#include "Fields.h"

#include "qpx_util.h"
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "custom_logger.h"


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

  std::vector<std::string> lines;
  boost::split(lines, str, boost::is_any_of("\n"));

  parse(lines);
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

void DaughterParser::modify_level(Level& currentLevel,
                                  const LevelRecord& l) const
{
  if (l.continuations_.count("MOME2"))
    currentLevel.set_q(parse_moment(l.continuations_.at("MOME2")));

  if (l.continuations_.count("MOMM1"))
    currentLevel.set_mu(parse_moment(l.continuations_.at("MOMM1")));
}


DecayScheme DaughterParser::get_decay(NuclideId daughter,
                                      std::string decay_name,
                                      double max_level_dif) const
{
  if (!nuclide_data_.count(daughter) ||
      !nuclide_data_.at(daughter).decays.count(decay_name))
    return DecayScheme();

  DecayData decaydata = nuclide_data_.at(daughter).decays.at(decay_name);

  Nuclide daughter_nuclide(decaydata.id.nuclide);

  UncertainDouble normalizeDecIntensToPercentParentDecay
      (1.0,1,UncertainDouble::SignMagnitudeDefined);
  UncertainDouble normalizeGammaIntensToPercentParentDecay
      (1.0,1,UncertainDouble::SignMagnitudeDefined);

  //multiple times???
  for (auto n : decaydata.norm)
  {
    if (n.NB.hasFiniteValue() && n.BR.hasFiniteValue())
      normalizeDecIntensToPercentParentDecay = n.NB * n.BR;
    if (n.NR.hasFiniteValue() && n.BR.hasFiniteValue())
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
    Level currentLevel(lev.energy, lev.spin_parity,
                       lev.halflife, lev.isomeric);

    modify_level(currentLevel, lev);

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
      if (e.intensity_total.hasFiniteValue())
        currentLevel.setFeedIntensity(e.intensity_total);
      else
        currentLevel.setFeedIntensity(e.intensity_beta_plus + e.intensity_ec);
    }

//    for (const CommentsRecord& c : lev.comments)
//    {

//    }


    daughter_nuclide.add_level(currentLevel);
  }

  for (const LevelRecord& lev : decaydata.levels)
  {
    for (const GammaRecord& g : lev.gammas)
    {
      Transition transition(g.energy,
                            g.intensity_rel_photons
                            * normalizeGammaIntensToPercentParentDecay);
      transition.set_multipol(g.multipolarity);
      transition.set_delta(g.mixing_ratio);
      transition.set_from(lev.energy);
      daughter_nuclide.add_transition_from(transition, max_level_dif);
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
      parent_nuclide.add_level(plv);
    }

    if (!parent_nuclide.empty() &&
        (parent_nuclide.levels().begin()->second.energy() > 0.0))
    {
      Level plv(Energy(0.0, UncertainDouble::SignMagnitudeDefined), SpinParity(), HalfLife());
      plv.setFeedingLevel(false);
      parent_nuclide.add_level(plv);
    }
  }

  DecayScheme ret(decay_name, parent_nuclide, daughter_nuclide,
                  decaydata.decay_info_, decaydata.reaction_info_);

  for (const HistoryRecord& h : decaydata.history)
  {
    json j;
    for (auto kvp : h.kvps)
      j[kvp.first] = kvp.second;
    ret.comments["history"].push_back(j);
  }

  for (const CommentsRecord& c : decaydata.comments)
    ret.comments["comments"].push_back(c.html());

  return ret;
}

DecayScheme DaughterParser::get_nuclide(NuclideId daughter, double max_level_dif) const
{
  if (!nuclide_data_.count(daughter))
    return DecayScheme();

  UncertainDouble normalizeDecIntensToPercentParentDecay
      (1.0,1,UncertainDouble::SignMagnitudeDefined);
  UncertainDouble normalizeGammaIntensToPercentParentDecay
      (1.0,1,UncertainDouble::SignMagnitudeDefined);

  LevelData nucdata = nuclide_data_.at(daughter).adopted_levels;

  Nuclide daughter_nuclide(daughter);

  // process normalization records
  if (nucdata.pnorm.valid())
  {
    if (nucdata.pnorm.NBBR.hasFiniteValue())
      normalizeDecIntensToPercentParentDecay = nucdata.pnorm.NBBR;
    if (nucdata.pnorm.NRBR.hasFiniteValue())
      normalizeGammaIntensToPercentParentDecay = nucdata.pnorm.NRBR;
  }

  for (const LevelRecord& lev : nucdata.levels)
  {
    Level currentLevel(lev.energy, lev.spin_parity,
                       lev.halflife, lev.isomeric);

    modify_level(currentLevel, lev);

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
      if (e.intensity_total.hasFiniteValue())
        currentLevel.setFeedIntensity(e.intensity_total);
      else
        currentLevel.setFeedIntensity(e.intensity_beta_plus + e.intensity_ec);
    }

//    for (const CommentsRecord& c : lev.comments)
//    {

//    }


    daughter_nuclide.add_level(currentLevel);
  }

  for (const LevelRecord& lev : nucdata.levels)
  {
    for (const GammaRecord& g : lev.gammas)
    {
      Transition transition(g.energy,
                            g.intensity_rel_photons
                            * normalizeGammaIntensToPercentParentDecay);
      transition.set_multipol(g.multipolarity);
      transition.set_delta(g.mixing_ratio);
      transition.set_from(lev.energy);
      daughter_nuclide.add_transition_from(transition, max_level_dif);
    }
  }

  DecayScheme ret(daughter.symbolicName() + " (adopted levels)",
                  Nuclide(), daughter_nuclide,
                  DecayInfo(), ReactionInfo());

  for (const HistoryRecord& h : nucdata.history)
  {
    json j;
    for (auto kvp : h.kvps)
      j[kvp.first] = kvp.second;
    ret.comments["history"].push_back(j);
  }

  for (const CommentsRecord& c : nucdata.comments)
    ret.comments["comments"].push_back(c.html());

  return ret;
}

DecayScheme DaughterParser::get_info() const
{
  DecayScheme ret("", Nuclide(), Nuclide(),
                  DecayInfo(), ReactionInfo());

  for (const HistoryRecord& h : mass_history_)
  {
    json j;
    for (auto kvp : h.kvps)
      j[kvp.first] = kvp.second;
    ret.comments["history"].push_back(j);
  }

  for (const CommentsRecord& c : mass_comments_)
    ret.comments["comments"].push_back(c.html());

  return ret;
}


std::list<BlockIndices> DaughterParser::find_blocks(const std::vector<std::string>& lines) const
{
  // create list of block boundaries
  // end index points behind last line of block!
  std::list<BlockIndices> blocks;
  size_t from = 0;
  boost::regex emptyline("^\\s*$");
  for (size_t i=0; i < lines.size(); ++i)
    if (boost::regex_match(lines.at(i), emptyline))
    {
      if (i-from > 1)
        blocks.push_back(BlockIndices(from, i));
      from = i + 1;
    }
  if (from < lines.size())
    blocks.push_back(BlockIndices(from, lines.size()));
  return blocks;
}

void DaughterParser::parse_comments_block(ENSDFData& i,
                                          std::list<HistoryRecord> &hist,
                                          std::list<CommentsRecord> &comm)
{
  while (i.has_more())
  {
    if (HistoryRecord::match(i.look_ahead()))
      hist.push_back(HistoryRecord(++i));
    else if (CommentsRecord::match(i.look_ahead(), "\\s"))
      comm.push_back(CommentsRecord(++i));
    else
    {
      DBG << "Unidentified record " << i.read_pop();
    }
  }
}

void DaughterParser::parse_reference_block(ENSDFData &i)
{
  while (i.has_more())
  {
    if (ReferenceRecord::match(i.look_ahead()))
    {
      auto ref = ReferenceRecord(++i);
      if (ref.valid())
        references_[ref.keynum] = ref.reference;
      else
        DBG << "Invalid reference " << ref.debug();
    }
    else
    {
      DBG << "Unidentified record " << i.read_pop();
    }
  }
}


void DaughterParser::parse(const std::vector<std::string>& lines)
{
  for (BlockIndices block_idx : find_blocks(lines))
  {
    ENSDFData d(lines, block_idx);
    auto header = IdRecord(d);

    ENSDFData data(lines, block_idx);
    if (!header.reflect_parse())
      DBG << "Bad header  ==  " << data.read();
    //    DBG << "IdRecord: " << header.extended_dsid;

    if (test(header.type & RecordType::Comments))
    {
      if (header.nuclide.composition_known())
        parse_comments_block(data,
                             nuclide_data_[header.nuclide].history,
                             nuclide_data_[header.nuclide].comments);
      else
        parse_comments_block(data,
                             mass_history_,
                             mass_comments_);
    }
    else if (test(header.type & RecordType::References) &&
        !header.nuclide.composition_known())
    {
      parse_reference_block(data);
    }
    else if (test(header.type & RecordType::AdoptedLevels))
    {
      LevelData lev(data);
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
      DecayData decaydata(data);

//      if (!decaydata.decay_info_.valid())
//        continue;

      if (!nuclide_data_.count(decaydata.id.nuclide))
        DBG << "No index for " << decaydata.id.nuclide.symbolicName()
            << " exists";
      else
      {
        nuclide_data_[decaydata.id.nuclide].merge_adopted(decaydata);
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
