#include "beta_record.h"
#include "ensdf_types.h"
#include <boost/regex.hpp>
#include "qpx_util.h"

bool BetaRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Za-z]{5}\\s\\sB\\s.*$");
}

BetaRecord
BetaRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return BetaRecord();
  auto line = data[idx];

  BetaRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.intensity = parse_norm_value(line.substr(21,8), line.substr(29,2));
  ret.LOGFT = parse_norm_value(line.substr(41,8), line.substr(49,6));

  ret.comment_flag = boost::trim_copy(line.substr(76,1));
  ret.uniquness = boost::trim_copy(line.substr(77,2));
  ret.quality = boost::trim_copy(line.substr(79,1));

  boost::regex filter("^[\\s0-9A-Za-z]{5}[02-9A-Za-z].B.*$");
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

std::string BetaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (intensity.hasFiniteValue())
    ret += " Intensity=" + intensity.to_string(true);
  if (LOGFT.hasFiniteValue())
    ret += " logFT=" + LOGFT.to_string(true);
  if (!comment_flag.empty())
    ret += " comment=" + comment_flag;
  if (!uniquness.empty())
    ret += " uniqueness=" + quality;
  if (!quality.empty())
    ret += " quality=" + quality;
  if (!continuation.empty())
    ret += "\n  Continuation:" + continuation;
  for (auto c : comments)
    ret += "\n  Comment: " + c.debug();
  return ret;
}
