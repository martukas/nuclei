#include "level_record.h"
#include "ensdf_types.h"
#include "qpx_util.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include "custom_logger.h"

bool LevelRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Za-z]{5}\\s\\sL\\s.*$");
}

LevelRecord
LevelRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return LevelRecord();
  auto line = data[idx];

  LevelRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.spin_parity = parse_spin_parity(line.substr(21, 18));
  ret.halflife = parse_halflife(line.substr(39, 16));
  ret.L = boost::trim_copy(line.substr(55,9));

  try
  {
    ret.S = parse_norm_value(line.substr(64,10), line.substr(74,2));
  }
  catch (...)
  {
    auto sstr = line.substr(64, 12);
    //      DBG << "<LevelRecord::parse> failed to parse Svalue=\'" << sstr << "\'";
  }

  ret.comment_flag = boost::trim_copy(line.substr(76,1));
  if (line[77] == 'M')
  {
    if (is_number(line.substr(78,1)))
      ret.isomeric = boost::lexical_cast<uint16_t>(line.substr(78,1));
    else
      ret.isomeric = 1;
  }

  ret.quality = boost::trim_copy(line.substr(79,1));

  boost::regex filter("^[\\s0-9A-Za-z]{5}[02-9A-Za-z@$].L.*$");
  while ((idx+1 < data.size()) &&
         (boost::regex_match(data[idx+1], filter) ||
          CommentsRecord::is(data[idx+1])))
  {
    ++idx;
    if (CommentsRecord::is(data[idx]))
      ret.comments.push_back(CommentsRecord::parse(idx, data));
    else
      ret.continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }

  return ret;
}

std::string LevelRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (isomeric)
    ret += " M" + std::to_string(isomeric);
  if (spin_parity.valid())
    ret += " SpinParity=" + spin_parity.to_string();
  if (halflife.isValid())
    ret += " Halflife=" + halflife.to_string(true);
  if (S.hasFiniteValue())
    ret += " S=" + S.to_string(true);
  if (!comment_flag.empty())
    ret += " comment=" + comment_flag;
  if (!quality.empty())
    ret += " quality=" + quality;
  if (!L.empty())
    ret += " L=" + L;
  if (!continuation.empty())
    ret += "\n  Continuation:" + continuation;
  for (auto c : comments)
    ret += "\n  Comment: " + c.debug();
  return ret;
}
