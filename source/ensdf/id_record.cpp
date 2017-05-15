#include "id_record.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "custom_logger.h"
#include "ensdf_types.h"

#include "ensdf_reaction_data.h"

bool IdRecord::is(const std::string& line)
{
  return match_record_type(line, "^[\\s0-9A-Za-z]{6}\\s{3}.*$");
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

void IdRecord::reflect_parse() const
{
  if (test(type & RecordType::Comments))
  {
    DBG << boost::trim_copy(extended_dsid)
        << " " << nuc_id.symbolicName();
  }
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
      DBG << "Unknown header type -- " << debug();
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
  else if (boost::contains(str, "(HI,XNG)"))
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
  ss << " " << nuc_id.symbolicName();
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
