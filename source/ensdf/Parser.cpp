#include "Parser.h"
#include "Reference.h"
#include "Fields.h"

#include "qpx_util.h"
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include "custom_logger.h"
#include "Translator.h"

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

Uncert DaughterParser::feed_norm(const ProdNormalizationRecord& pnorm,
                                 std::vector<NormalizationRecord> norm)
{
  Uncert ret(1.0,1,Uncert::SignMagnitudeDefined);

  //multiple times???
  for (auto n : norm)
    if (n.NB.hasFiniteValue() && n.BR.hasFiniteValue())
      ret = n.NB * n.BR;

  if (pnorm.NBBR.hasFiniteValue())
    ret = pnorm.NBBR;

  return ret;
}

Uncert DaughterParser::gamma_norm(const ProdNormalizationRecord& pnorm,
                                  std::vector<NormalizationRecord> norm)
{
  Uncert ret(1.0,1,Uncert::SignMagnitudeDefined);

  //multiple times???
  for (auto n : norm)
    if (n.NR.hasFiniteValue() && n.BR.hasFiniteValue())
      ret = n.NR * n.BR;

  if (pnorm.NRBR.hasFiniteValue())
    ret = pnorm.NRBR;

  return ret;
}

Level DaughterParser::construct_level(const LevelRecord& record,
                                      Uncert intensity_norm)
{
  Level ret(record.energy, record.spin_parity,
            record.halflife, record.isomeric);

  for (const AlphaRecord& a : record.alphas)
    if (a.intensity_alpha.hasFiniteValue())
      ret.setFeedIntensity(a.intensity_alpha * intensity_norm);

  for (const BetaRecord& b : record.betas)
    if (b.intensity.hasFiniteValue())
      ret.setFeedIntensity(b.intensity * intensity_norm);

  for (const ECRecord& e : record.ECs)
    if (e.intensity_total.hasFiniteValue())
      ret.setFeedIntensity(e.intensity_total);
    else
      ret.setFeedIntensity(e.intensity_beta_plus + e.intensity_ec);

//  if (record.continuations_.count("MOME2"))
//    ret.set_q(parse_moment(record.continuations_.at("MOME2")));

//  if (record.continuations_.count("MOMM1"))
//    ret.set_mu(parse_moment(record.continuations_.at("MOMM1")));

  json vals;
  if (record.offsets.size())
  {
    std::string offsets;
    for (auto o : record.offsets)
      offsets += o + " ";
    vals.push_back("<b>Offsets:</b> " + offsets);
  }
  if (record.spin_parity.valid())
    vals.push_back("<b>Spin & parity:</b> "
                   + record.spin_parity.to_string());
  if (record.halflife.isValid())
    vals.push_back("<b>Halflife:</b> "
                   + record.halflife.to_string());
  if (record.isomeric)
    vals.push_back("<b>Isomeric level:</b> "
                   + record.isomeric);
  if (record.L.size())
    vals.push_back("<b>Angular momentum:</b> "
                   + record.L);
  if (record.S.defined())
    vals.push_back("<b>Spectroscopic strength:</b> "
                   + record.S.to_string(true));
  if (!record.comment_flag.empty())
    vals.push_back("<b>Comment flag:</b> " + record.comment_flag);
  if (!record.quality.empty())
    vals.push_back("<b>Quality:</b> " + record.quality);
  ret.add_text("Values", vals);

  if (record.continuations_.size())
  {
    json cont;
    for (const auto& c : record.continuations_)
      cont.push_back("<b>" + c.first + "</b>: " + c.second.value_refs());
    ret.add_text("Continued...", cont);
  }

  if (record.comments.size())
  {
    json comments;
    for (const CommentsRecord& c : record.comments)
      comments.push_back(c.html());
    ret.add_text("Comments", comments);
  }

  json extras;
  for (auto a : record.alphas)
    extras.push_back(Translator::instance().spaces_to_html_copy(a.debug()));
  for (auto a : record.betas)
    extras.push_back(Translator::instance().spaces_to_html_copy(a.debug()));
//  for (auto a : record.gammas)
//    extras.push_back(Translator::instance().spaces_to_html_copy(a.debug()));
  for (auto a : record.ECs)
    extras.push_back(Translator::instance().spaces_to_html_copy(a.debug()));
  for (auto a : record.particles)
    extras.push_back(Translator::instance().spaces_to_html_copy(a.debug()));
  if (!extras.empty())
    ret.add_text("Extras", extras);

  return ret;
}

Transition DaughterParser::construct_transition(const GammaRecord& record,
                                                Uncert intensity_norm)
{
  Transition ret(record.energy,
                 record.intensity_rel_photons * intensity_norm);
  ret.set_multipol(record.multipolarity);
  ret.set_delta(record.mixing_ratio);

  json vals;
  if (record.intensity_rel_photons.defined())
    vals.push_back("<b>Relative photon intensity:</b> "
                   + record.intensity_rel_photons.to_string(true));
  if (record.intensity_total_transition.defined())
    vals.push_back("<b>Total transition intensity:</b> "
                   + record.intensity_total_transition.to_string(true));
  if (record.mixing_ratio.defined())
    vals.push_back("<b>Mixing ratio:</b> "
                   + record.mixing_ratio.to_string(true));
  if (record.conversion_coef.defined())
    vals.push_back("<b>Conversion coefficient:</b> "
                   + record.conversion_coef.to_string(true));
  if (!record.comment_flag.empty())
    vals.push_back("<b>Comment flag:</b> " + record.comment_flag);
  if (!record.coincidence.empty())
    vals.push_back("<b>Coincidence:</b> " + record.coincidence);
  if (!record.quality.empty())
    vals.push_back("<b>Quality:</b> " + record.quality);
  ret.add_text("Values", vals);

  if (record.continuations_.size())
  {
    json cont;
    for (const auto& c : record.continuations_)
      cont.push_back("<b>" + c.first + "</b>: " + c.second.value_refs());
    ret.add_text("Continued...", cont);
  }

  if (record.comments.size())
  {
    json comments;
    for (const CommentsRecord& c : record.comments)
      comments.push_back(c.html());
    ret.add_text("Comments", comments);
  }

  return ret;
}

Nuclide DaughterParser::construct_parent(const std::vector<ParentRecord>& parents)
{
  // is this really the best way to deal with multiple parents?

  if (parents.empty())
    return Nuclide();

  Nuclide ret = Nuclide(parents.at(0).nuclide);

  json comm;
  for (ParentRecord p : parents)
  {
    ret.addHalfLife(p.hl);

    Level plv(p.energy, p.spin, p.hl);
    plv.setFeedingLevel(true);
    ret.add_level(plv);

    comm.push_back(p.debug());
  }

  if (!comm.empty())
    ret.add_text("Parent records", comm);

  if (!ret.empty() &&
      (ret.levels().begin()->second.energy() > 0.0))
  {
    Level plv(Energy(0.0, Uncert::SignMagnitudeDefined), SpinParity(), HalfLife());
    plv.setFeedingLevel(false);
    ret.add_level(plv);
  }

  return ret;
}

void DaughterParser::add_text(DecayScheme& scheme,
                              const std::list<HistoryRecord>& hist,
                              const std::list<CommentsRecord>& comm) const
{
  if (!comm.empty())
  {
    json comments;
    for (const CommentsRecord& c : comm)
      comments.push_back(c.html());
    scheme.add_text("Comments", comments);
  }

  if (!hist.empty())
  {
    json history;
    for (const HistoryRecord& h : hist)
      for (const auto& kvp : h.kvps)
        history.push_back("<b>" + kvp.first + ":</b> " + kvp.second);
    scheme.add_text("History", history);
  }

  for (auto r : references_)
  {
    scheme.insert_reference(r.first);
    scheme.insert_reference(CommentsRecord::adjust_case(r.first));
  }
}


DecayScheme DaughterParser::get_decay(NuclideId daughter,
                                      std::string decay_name,
                                      double max_level_dif) const
{
  if (!nuclide_data_.count(daughter) ||
      !nuclide_data_.at(daughter).decays.count(decay_name))
    return DecayScheme();

  DecayData decaydata = nuclide_data_.at(daughter).decays.at(decay_name);
  Uncert feed_n = feed_norm(decaydata.pnorm, decaydata.norm);
  Uncert gamma_n = gamma_norm(decaydata.pnorm, decaydata.norm);

  Nuclide daughter_nuclide(decaydata.id.nuclide);

  for (const LevelRecord& lev : decaydata.levels)
    daughter_nuclide.add_level(construct_level(lev, feed_n));

  for (const LevelRecord& lev : decaydata.levels)
    for (const GammaRecord& g : lev.gammas)
    {
      Transition transition
          = construct_transition(g, gamma_n);
      transition.set_from(lev.energy);
      daughter_nuclide.add_transition_from(transition, max_level_dif);
    }

  Nuclide parent_nuclide = construct_parent(decaydata.parents);

  DecayScheme ret(decay_name, parent_nuclide, daughter_nuclide,
                  decaydata.decay_info_, decaydata.reaction_info_);

  add_text(ret, decaydata.history, decaydata.comments);

  return ret;
}

DecayScheme DaughterParser::get_nuclide(NuclideId daughter, double max_level_dif) const
{
  if (!nuclide_data_.count(daughter))
    return DecayScheme();

  LevelData nucdata = nuclide_data_.at(daughter).adopted_levels;
  std::vector<NormalizationRecord> dummy;
  Uncert feed_n = feed_norm(nucdata.pnorm, dummy);
  Uncert gamma_n = gamma_norm(nucdata.pnorm, dummy);

  Nuclide daughter_nuclide(daughter);
  for (const LevelRecord& lev : nucdata.levels)
    daughter_nuclide.add_level(construct_level(lev, feed_n));

  for (const LevelRecord& lev : nucdata.levels)
    for (const GammaRecord& g : lev.gammas)
    {
      Transition transition
          = construct_transition(g, gamma_n);
      transition.set_from(lev.energy);
      daughter_nuclide.add_transition_from(transition, max_level_dif);
    }

  DecayScheme ret(daughter.symbolicName() + " (adopted levels)",
                  Nuclide(), daughter_nuclide,
                  DecayInfo(), ReactionInfo());

  add_text(ret, nucdata.history, nucdata.comments);

  return ret;
}

DecayScheme DaughterParser::get_info() const
{
  DecayScheme ret("", Nuclide(), Nuclide(),
                  DecayInfo(), ReactionInfo());

  add_text(ret, mass_history_, mass_comments_);
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
