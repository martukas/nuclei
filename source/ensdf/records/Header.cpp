#include <ensdf/records/Header.h>
#include <ensdf/Fields.h>

#include <boost/algorithm/string.hpp>
#include <sstream>
#include "qpx_util.h"
#include <NucData/ReactionInfo.h>

#include <util/logger.h>

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
      year = std::stoi(boost::trim_copy(year_str));
    else
      DBG("<IdRecord> Cannot intepret year {}", line);
  }
  std::string month_str = line.substr(78, 2);
  if (!boost::trim_copy(month_str).empty())
  {
    if (is_number(boost::trim_copy(month_str)))
      month = std::stoi(boost::trim_copy(month_str));
    else
      DBG("<IdRecord> Cannot intepret month {}", line);
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

RecordType IdRecord::is_type(std::string s)
{
  std::string str = boost::to_upper_copy(s);
  if (boost::contains(str, "COMMENTS"))
    return RecordType::Comments;
  else if (boost::contains(str, "REFERENCES"))
    return RecordType::References;
  else if (boost::contains(str, "ADOPTED LEVELS"))
    return RecordType::AdoptedLevels;
//  else if (ReactionInfo(str, NuclideId()).valid()
//           || DecayInfo(str).valid())
  return RecordType::ReactionDecay;
//  return RecordType::Invalid;
}

std::string IdRecord::type_to_str(RecordType t)
{
  if (test(t & RecordType::Comments))
    return "COMMENTS";
  else if (test(t & RecordType::References))
    return "REFERENCES";
  else if (test(t & RecordType::AdoptedLevels))
    return "ADOPTED LEVELS";
  if (test(t & RecordType::ReactionDecay))
    return "REACTION / DECAY";
  return "INVALID";
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
