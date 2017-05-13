#include "ensdf_records.h"
#include "ensdf_types.h"

#include "custom_logger.h"
#include "qpx_util.h"

#include <boost/regex.hpp>

bool match_record_type(const std::string& line,
                       const std::string& pattern)
{
  boost::regex filter(pattern);
  return (line.size() == 80) &&
      boost::regex_match(line, filter);
}

/**
 * @brief DaughterParser::extractContinuationRecords
 * @param adoptedblock block to search continuation records in
 * @param requestedRecords list of requested records
 * @param typeOfContinuedRecord type of record (default: L(evel))
 * @return list of found records (same size as requestedRecords, empty strings if no record was found)
 */
std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
                                                    const std::list<std::string> &requestedRecords,
                                                    const std::vector<std::string> &data,
                                                    std::string typeOfContinuedRecord)
{
  // fetch records
  boost::regex crecre("^[A-Z0-9\\s]{5,5}[A-RT-Z0-9] "
                      + typeOfContinuedRecord + " (.*)$");
  std::vector<std::string> crecs;
  for (size_t i = adoptedblock.first; i < adoptedblock.last; ++i)
  {
    boost::smatch what;
    if (boost::regex_search(data.at(i), what, crecre) &&
        (what.size() > 1))
      crecs.push_back(data.at(i));
  }
  std::vector<std::string> crecs2;
  // remove record id from beginning of string
  for (size_t i=0; i<crecs.size(); i++)
  {
    crecs[i].erase(0, 9);
    crecs2.push_back(crecs.at(i));
  }
  // join lines and then split records
  std::string tmp = join(crecs2, "$");
  boost::split(crecs2, tmp, boost::is_any_of("$"));
  for (size_t i=0; i<crecs2.size(); i++)
    crecs2[i] = boost::trim_copy(crecs2[i]);
  // search and parse requested fields
  std::vector<std::string> result;
  for ( auto &req : requestedRecords)
  {
    std::string rstr;
    for (size_t i=0; i<crecs2.size(); i++)
    {
      if ((crecs2.at(i).size() >= req.size()) &&
          (crecs2.at(i).substr(0, req.size()) == req))
      {
        rstr = boost::trim_copy(crecs2.at(i).substr(5, crecs2.at(i).size() - 5));
        break;
      }
    }
    if (!rstr.empty() && (rstr[0] == '='))
      rstr = rstr.substr(1, rstr.size()-1);
    result.push_back(rstr);
  }
  return result;
}

bool is_gamma_line(const std::string& line,
                   std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  G "))) ||
      (!nucid2.empty() && is_gamma_line(line, nucid2));
}

bool is_level_line(const std::string& line,
                   std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  L ")))||
      (!nucid2.empty() && is_level_line(line, nucid2));
}

bool is_intensity_line(const std::string& line,
                       std::string nucid,
                       std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  E ")))||
      (!nucid2.empty() && is_intensity_line(line, nucid2));
}

bool is_norm_line(const std::string& line,
                  std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  N ")))||
      (!nucid2.empty() && is_norm_line(line, nucid2));
}

bool is_p_norm_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + " PN ")))||
      (!nucid2.empty() && is_p_norm_line(line, nucid2));
}

bool is_feed_a_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  A ")))||
      (!nucid2.empty() && is_feed_a_line(line, nucid2));
}

bool is_feed_b_line(const std::string& line,
                    std::string nucid, std::string nucid2)
{
  return ((line.size() > 8) &&
          (line.substr(0,9) == (nucid + "  B ")))||
      (!nucid2.empty() && is_feed_b_line(line, nucid2));
}

bool is_feed_line(const std::string& line,
                  std::string nucid, std::string nucid2)
{
  return is_feed_a_line(line, nucid, nucid2) ||
      is_feed_b_line(line, nucid, nucid2);
}

std::map<Energy, std::string> get_gamma_lines(const std::vector<std::string>& data,
                                              BlockIndices bounds,
                                              NuclideId nucid)
{
  std::map<Energy, std::string> e2g;
  std::string dNucid1 = nid_to_ensdf(nucid, true);
  std::string dNucid2 = nid_to_ensdf(nucid, false);
  for (size_t i = bounds.first; i < bounds.last; ++i)
    if (is_gamma_line(data[i], dNucid1, dNucid2))
    {
      Energy gk = parse_energy(data[i].substr(9, 12));
      if (gk.valid())
        e2g[gk] = data[i];
    }
  return e2g;
}

IdRecord IdRecord::parse(size_t& idx,
                         const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return IdRecord();

  auto line = data[idx];

  IdRecord ret;

  ret.nuc_id = parse_check_nid(line.substr(0, 5));
  ret.extended_dsid = ret.dsid = line.substr(9, 30);
  ret.dsref = line.substr(39, 15);
  ret.pub = line.substr(65, 8);
  std::string year_str = line.substr(74, 4);
  if (!boost::trim_copy(year_str).empty())
    ret.year = boost::lexical_cast<uint16_t>(year_str);
  std::string month_str = line.substr(78, 2);
  if (!boost::trim_copy(month_str).empty())
    ret.month = boost::lexical_cast<uint16_t>(month_str);

  if (ret.dsid.size() && ret.dsid.at(ret.dsid.size()-1) == ',')
    ret.extended_dsid += parse(++idx, data).extended_dsid;

  ret.type = is_type(ret.extended_dsid);
  return ret;
}

HistoryRecord HistoryRecord::parse(size_t& idx,
                                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return HistoryRecord();
  auto line = data[idx];
  HistoryRecord ret;

  ret.nuc_id = parse_check_nid(line.substr(0, 5));

  boost::regex filter("^[\\s0-9A-Z]{5}[02-9A-Za-z]\\sH.*$");
  std::string hdata = line.substr(9, 71);
  while ((idx+1 < data.size()) &&
         boost::regex_match(data[idx+1], filter))
    hdata += data[++idx].substr(9,71);

  std::vector<std::string> tokens;
  boost::split(tokens, hdata, boost::is_any_of("$"));
  for (auto t : tokens)
  {
    boost::trim(t);
    std::vector<std::string> tokens2;
    boost::split(tokens2, t, boost::is_any_of("="));
    if (tokens2.size() > 1)
      ret.kvps[boost::trim_copy(tokens2[0])] =
          boost::trim_copy(tokens2[1]);
  }
  return ret;
}

void IdRecord::reflect_parse() const
{
  if (test(type & RecordType::Comments))
  {  }
  else if (test(type & RecordType::References))
  {  }
  else if (test(type & RecordType::CoulombExcitation))
  {  }
  else if (test(type & RecordType::MuonicAtom))
  {  }
  else if (test(type & RecordType::Reaction))
  {  }
  else if (test(type & RecordType::HiXng))
  {  }
  else if (test(type & RecordType::AdoptedLevels))
  {  }
  else if (test(type & RecordType::Decay) ||
           (test(type & RecordType::InelasticScattering)))
  {  }
  else
  {
      DBG << "Unknown header -- " << debug();
  }
}

RecordType IdRecord::is_type(std::string s)
{
  std::string str = boost::to_upper_copy(s);
  if (boost::contains(str, "COMMENTS"))
    return RecordType::Comments;
  else if (boost::contains(str, "REFERENCES"))
    return RecordType::References;
  else if (boost::contains(str, "MUONIC ATOM"))
    return RecordType::MuonicAtom;
  else if (boost::contains(str, "(HI,"))
    return RecordType::HiXng;

  RecordType ret = RecordType::Invalid;
  if (ReactionData::match(s))
    ret |= RecordType::Reaction;
  if (boost::contains(str, "INELASTIC SCATTERING"))
    ret |= RecordType::InelasticScattering;
  if (boost::contains(str, "COULOMB EXCITATION"))
    ret |= RecordType::CoulombExcitation;
  if (boost::contains(str, "ADOPTED LEVELS"))
    ret |= RecordType::AdoptedLevels;
  if (boost::contains(str, "TENTATIVE"))
    ret |= RecordType::Tentative;
  if (boost::contains(str, "GAMMAS"))
    ret |= RecordType::Gammas;
  if (boost::contains(str, "DECAY"))
    ret |= RecordType::Decay;

  return ret;
}

std::string IdRecord::type_to_str(RecordType t)
{
  if (test(t & RecordType::Comments))
    return "COMMENTS";
  else if (test(t & RecordType::References))
    return "REFERENCES";
  else if (test(t & RecordType::MuonicAtom))
    return "MUONIC ATOM";
  else if (test(t & RecordType::Invalid))
    return "INVALID";
  else if (test(t & RecordType::HiXng))
    return "(HI,XNG)";

  std::string ret;

  if (test(t & RecordType::AdoptedLevels))
    ret += "ADOPTED LEVELS";
  if (test(t & RecordType::CoulombExcitation))
    ret += "COULOMB EXCITATION";
  if (test(t & RecordType::InelasticScattering))
    ret += "INELASTIC SCATTERING";
  if (test(t & RecordType::Reaction))
    ret += "REACTION";
  if (test(t & RecordType::Decay))
    ret += "DECAY";
  if (test(t & RecordType::Gammas))
    ret += ", GAMMAS";
  if (test(t & RecordType::Tentative))
    ret += ": TENTATIVE";

  return ret;
}

std::string IdRecord::debug() const
{
  std::stringstream ss;
  ss << type_to_str(type);
  ss << " " << nuc_id.verboseName();
  ss << " \"" << boost::trim_copy(extended_dsid) << "\"";
  ss << " dsref=\"" << dsref << "\"";
  ss << " pub=\"" << pub << "\"  ";
  ss << year << "/" << month;
//  ss << " dsid=\"" << boost::trim_copy(dsid) << "\"";
//  if (continued)
//    ss << " CONTINUED ";
//  if (numlines)
//    ss << " lines=" << numlines;
  return ss.str();
}

CommentsRecord CommentsRecord::from_id(const IdRecord &record,
                                       BlockIndices block)
{
  if ((record.type & RecordType::Comments) == RecordType::Invalid)
  {
    DBG << "Bad ID record type " << record.debug();
    return CommentsRecord();
  }

  CommentsRecord ret;
  ret.block = block;
  ret.nuclide = record.nuc_id;

  return ret;
}


ParentRecord ParentRecord::from_ensdf(const std::string &record)
{
  ParentRecord prec;
  if (record.size() < 50)
    return prec;
  prec.nuclide = parse_nid(record.substr(0,5));
  prec.energy = parse_energy(record.substr(9, 12));
  prec.hl = parse_halflife(record.substr(39, 16));
  prec.spin = parse_spin_parity(record.substr(21, 18));
  return prec;
}

std::string ParentRecord::to_string() const
{
  std::string ret;
  ret = nuclide.verboseName()
      + " " + energy.to_string()
      + " " + spin.to_string()
      + " " + hl.to_string();
  return ret;
}

BasicDecayData BasicDecayData::from_id(const IdRecord &record,
                                       BlockIndices block)
{
  if (((record.type & RecordType::Decay) == RecordType::Invalid) &&
      ((record.type & RecordType::InelasticScattering) == RecordType::Invalid))
  {
    DBG << "Bad ID record type " << record.debug();
    //HACK (Tentative should propagate to NucData)
    return BasicDecayData();
  }

  BasicDecayData ret;
  ret.dsid = record.dsid; // saved for comparison with xref records
  ret.block = block;
  ret.daughter = record.nuc_id;

  auto dsid = record.extended_dsid;
  std::string parents; bool valid = false; bool space = false;
  for (size_t i = 0; i < dsid.size(); ++i)
  {
    if (dsid.at(i) != ' ')
      valid = true;
    if (valid && (dsid.at(i) == ' '))
      space = true;
    if (valid && space && (dsid.at(i) != ' '))
      break;
    parents += std::string(1, dsid.at(i));
  }

  boost::replace_all(dsid, "DECAY", "");
  std::vector<std::string> tokens;
  boost::split(tokens, dsid, boost::is_any_of(" "));
  if (tokens.size() < 2)
    return BasicDecayData();

  std::string parent_xx = "([0-9]+[A-Z]*(?:\\([A-Z0-9]+,[A-Z0-9]+\\))?(?::[\\s0-9A-Z]+)?(?:\\[\\+[0-9]+\\])?)";

  boost::regex parents_expr("^(?:\\s)*" + parent_xx + "(?:,[\\s]*" + parent_xx + ")*(?:\\s)*$");
  boost::smatch what;
  if (boost::regex_match(parents, what, parents_expr) && (what.size() > 1))
  {
    for (size_t i=1; i < what.size(); ++i)
    {
      std::string parent_str = what[1];
//      DBG << "Parent string " << parent_str;
      boost::regex parent_expr("^([0-9]+[A-Z]*)(\\([A-Z0-9]+,[A-Z0-9]+\\))?(:[\\s0-9A-Z]+)?(\\[\\+[0-9]+\\])?$");
      boost::smatch what2;
      if (boost::regex_match(parent_str, what2, parent_expr) && (what2.size() > 1))
      {
        ret.parent = parse_nid(what2[1]);
        if (!check_nid_parse(what2[1], ret.parent))
        {
          ERR << "<BasicDecay> Could not parse parent NucID   \""
//              << boost::trim_copy(nid_to_ensdf(ret.parent)) << "\"  !=  \""
              << what2[1] << "\" from \"" << parent_str << "\""
              << "   in   \"" << record.extended_dsid << "\"";
      //    return BasicDecayData();
        }


      }
      else
        DBG << "Failed parent " << parent_str;

    }
  }
//  else
//    DBG << "Failed parents " << parents;

  ret.mode = parse_decay_mode(tokens.at(1));
  if (mode_to_ensdf(ret.mode) != tokens.at(1))
  {
//    ERR << "Could not parse decay mode in \"" << record.extended_dsid << "\"";
    return BasicDecayData();
  }

  if (tokens.size() > 2)
  {
    ret.hl = parse_halflife(tokens.at(2));

  }

  return ret;
}

//BasicDecayData BasicDecayData::from_ensdf(const std::string &header, BlockIndices block)
//{
//  BasicDecayData decaydata;

//  boost::regex decay_expr("^([\\sA-Z0-9]{5,5})\\s{4,4}[0-9]{1,3}[A-Z]{1,2}\\s+((?:B-|B\\+|EC|IT|A)\\sDECAY).*$");
//  boost::smatch what;
//  if (!boost::regex_match(header, what, decay_expr) || (what.size() < 2))
//    return decaydata;

//  decaydata.daughter = parse_nid(what[1]);
//  decaydata.mode = parseDecayType(what[2]);
//  decaydata.block = block;
//  decaydata.dsid = boost::trim_copy(substr(9,30)); // saved for comparison with xref records
//  return decaydata;
//}

DecayMode BasicDecayData::parseDecayType(const std::string &tstring)
{
  DecayMode mode;
  if (tstring == "EC DECAY")
    mode.set_electron_capture(1);
  if (tstring == "B+ DECAY")
    mode.set_beta_plus(1);
  if (tstring == "B- DECAY")
    mode.set_beta_minus(1);
  if (tstring == "IT DECAY")
    mode.set_isomeric(true);
  if (tstring == "A DECAY")
    mode.set_alpha(true);
  return mode;
}

std::string BasicDecayData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName();
  ret += "   mode=" + mode.to_string();
//  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (hl.isValid())
    ret += "   hl=" + hl.to_string(true);
  ret += "   dsid=\"" + dsid + "\"";
  return ret;
}

ReactionData ReactionData::from_id(const IdRecord &record,
                                   BlockIndices block)
{
  ReactionData ret;
  ret.dsid = record.dsid;
  ret.block = block;
  ret.daughter = record.nuc_id;

  std::string nuclide = "[\\w\\d]+";
  std::string reactant = "[\\s\\w\\d'\\+-]+";
  std::string in_out = "\\(" + reactant + "," + reactant + "\\)";
  std::string target_reactions = nuclide + "\\s*" + in_out + "(?:\\s*,\\s*" + in_out + ")*";
  std::string splitxtras = "^(" + target_reactions + "(?:\\s*,\\s*" + target_reactions + "\\s*)*)(.*)$";

  std::string xtions, extras;
//  std::string xtras;

//  DBG << "Will parse " << record.debug();

  boost::regex xtions_xtras(splitxtras);
  boost::smatch what;
  if (boost::regex_match(boost::trim_copy(record.extended_dsid), what, xtions_xtras) && (what.size() > 1))
  {
//    DBG << " good";
    xtions = what[1];
    if (what.size() > 2)
      extras = what[2];
  }

//  DBG << " Xions " << xtions;

  std::string split_xxtions = "^\\s*(" + target_reactions + ")(?:\\s*,\\s(" + target_reactions + "))*.*$";
  boost::regex xtions_exp(split_xxtions);
  boost::smatch what2;
  boost::regex_match(xtions, what2, xtions_exp);
  for (size_t i=1; i < what2.size(); ++i)
  {
    std::string parse_xtions = "^(" + nuclide + ")\\s*(" + in_out + ")(?:\\s*,\\s*(" + in_out + "))*$";
    boost::regex xtion_exp(parse_xtions);
    boost::smatch what3;

//    DBG << "  xoion \"" << what2[i] << "\"";
    std::string layout = what2[i];
    std::string layout2;
    for (size_t z =0; z < layout.size(); ++z)
      layout2 += "\"" + std::string(1,layout.at(z)) + "\" ";

//    DBG << "        " << layout2;

    if (boost::regex_match(std::string(what2[i]), what3, xtion_exp) && (what3.size() > 2))
    {
//      for (auto xx =0; xx < what3.size(); ++xx)
//        DBG << "    " << xx << "=" << what3[xx];

      Reaction reaction;
//      DBG << "   nucid=" << what3[1];
      reaction.target = parse_nid(what3[1]);
      if (!reaction.target.composition_known() && reaction.target.Z())
        reaction.target.set_A(ret.daughter.A());

      for (size_t j=2; j < what3.size(); ++j)
      {
//        DBG << "   ants " << what3[j];

        std::string variant = "^\\((" + reactant + "),(" + reactant + ")\\)$";
        boost::regex variant_exp(variant);
        boost::smatch what4;
        if (boost::regex_match(std::string(what3[j]), what4, variant_exp) && (what4.size() > 2))
        {
          Reactants reactants;
          reactants.in = what4[1];
          reactants.out = what4[2];
          reaction.variants.push_back(reactants);
        }
      }
      ret.reactions.push_back(reaction);
    }
  }

  if (ret.find_remove(extras, "E=", "E=") ||
      ret.find_remove(extras, "E =", "E =") ||
      ret.find_remove(extras, "E<", "E") ||
      ret.find_remove(extras, "E>", "E") ||
      ret.find_remove(extras, "E AP", "E "))
  {
    boost::trim(ret.energy);
    std::size_t found = ret.energy.find("E=");
    if (found!=std::string::npos)
      ret.energy.erase(found, 2);
    else
      ret.energy.erase(0,1);
    boost::trim(ret.energy);
  }

  boost::trim(extras);
  std::size_t found = ret.energy.find(":");
  if (found!=std::string::npos)
    ret.energy.erase(found, 1);
  boost::trim(extras);

  ret.qualifier = extras;

  return ret;
}

bool ReactionData::find_remove(std::string& extras, std::string wanted, std::string trim_what)
{
  std::size_t found = extras.find(wanted);
  if (found!=std::string::npos)
  {
    energy = extras.substr(found, extras.size()-found);
    extras.erase(found, extras.size()-found);
    found = energy.find(trim_what);
    if (found!=std::string::npos)
      energy.erase(found, trim_what.size());
    boost::trim(energy);
    return true;
  }
  return false;
}


std::string ReactionData::Reactants::to_string() const
{
  return "(" + in + "," + out + ")";
}

std::string ReactionData::Reaction::to_string() const
{
  std::string ret = boost::trim_copy(nid_to_ensdf(target, false));
  bool first = true;
  for (auto x : variants)
  {
    if (!first)
      ret += ",";
    if (first)
      first = false;
    ret += x.to_string();
  }
  return ret;
}

std::string ReactionData::to_string() const
{
  std::string ret;
  ret = daughter.verboseName();
  ret += "   dsid=\"" + dsid + "\"  ";
  bool first = true;
  for (auto r : reactions)
  {
    if (!first)
      ret += ",";
    if (first)
      first = false;
    ret += r.to_string();
  }
  ret += "  q=\"" + qualifier + "\"  ";
//  if (!energy.empty())
  ret += energy;
  return ret;
}

bool ReactionData::match(std::string record)
{
  std::string nuclide = "[\\w\\d]+";
  std::string reactant = "[\\s\\w\\d'\\+-]+";
  std::string in_out = "\\(" + reactant + "," + reactant + "\\)";
  std::string target_reactions = nuclide + "\\s*" + in_out + "(?:\\s*,\\s*" + in_out + ")*";
  std::string splitxtras = "^(" + target_reactions + "(?:\\s*,\\s*" + target_reactions + "\\s*)*)(.*)$";

  boost::smatch what;
  boost::regex filter(splitxtras);
  return boost::regex_match(boost::trim_copy(record), what, filter);
}
