#include "Header.h"
#include "Fields.h"

#include <boost/algorithm/string.hpp>
#include <sstream>
#include "qpx_util.h"
#include "ReactionInfo.h"

#include "custom_logger.h"

bool IdRecord::match(const std::string& line)
{
  return match_first(line, "\\s{3}");
}

IdRecord::IdRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_check_nid(line.substr(0, 5));
  extended_dsid = dsid = boost::trim_copy(line.substr(9, 30));
  dsref = line.substr(39, 15);
  pub = line.substr(65, 8);
  std::string year_str = line.substr(74, 4);
  if (!boost::trim_copy(year_str).empty())
  {
    if (is_number(boost::trim_copy(year_str)))
      year = boost::lexical_cast<uint16_t>(boost::trim_copy(year_str));
    else
      DBG << "<IdRecord> Cannot intepret year " << line;
  }
  std::string month_str = line.substr(78, 2);
  if (!boost::trim_copy(month_str).empty())
  {
    if (is_number(boost::trim_copy(month_str)))
      month = boost::lexical_cast<uint16_t>(boost::trim_copy(month_str));
    else
      DBG << "<IdRecord> Cannot intepret month " << line;
  }

  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (match_cont(line2, "\\s{2}"))
      extended_dsid += boost::trim_copy(i.read_pop().substr(9, 30));
    else if (CommentsRecord::match(line2))
      comments.push_back(CommentsRecord(++i));
    else
      break;
  }

  type = is_type(extended_dsid);
}

bool IdRecord::reflect_parse() const
{
  if (test(type & RecordType::Comments))
  {
//    DBG << boost::trim_copy(extended_dsid)
//        << " " << nuclide.symbolicName();
  }
  else if (test(type & RecordType::References))
  {  }
  else if (test(type & RecordType::CoulombExcitation))
  {  }
  else if (test(type & RecordType::IsomericTransition))
  {  }
  else if (test(type & RecordType::NeutronResonances))
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
      DBG << "Unknown header type -- " << debug();
      return false;
  }
  return true;
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
  if (ReactionInfo::match(s))
    ret |= RecordType::Reaction;
  if (boost::contains(str, "INELASTIC SCATTERING"))
    ret |= RecordType::InelasticScattering;
  if (boost::contains(str, "COULOMB EXCITATION"))
    ret |= RecordType::CoulombExcitation;
  if (boost::contains(str, "ADOPTED LEVELS"))
    ret |= RecordType::AdoptedLevels;
  if (boost::contains(str, "TENTATIVE"))
    ret |= RecordType::Tentative;
  if (boost::contains(str, "ISOMERIC TRANSITION"))
    ret |= RecordType::IsomericTransition;
  if (boost::contains(str, "NEUTRON RESONANCES"))
    ret |= RecordType::NeutronResonances;
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
  if (test(t & RecordType::IsomericTransition))
    ret += "ISOMERIC TRANSITION";
  if (test(t & RecordType::NeutronResonances))
    ret += "NEUTRON RESONANCES";
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
  ss << " " << nuclide.symbolicName() << "  ID   ";
  ss << type_to_str(type);
  ss << " \"" << boost::trim_copy(extended_dsid) << "\"";
  ss << " dsref=\"" << dsref << "\"";
  ss << " pub=\"" << pub << "\"  ";
  ss << year << "/" << month;
  for (auto c : comments)
    ss << "\n      Comment: " << c.debug();
//  ss << " dsid=\"" << boost::trim_copy(dsid) << "\"";
//  if (continued)
//    ss << " CONTINUED ";
//  if (numlines)
//    ss << " lines=" << numlines;
  return ss.str();
}

bool IdRecord::valid() const
{
  return nuclide.valid();
}
