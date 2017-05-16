#include "beta_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

bool BetaRecord::match(const std::string& line)
{
  return match_first(line, "\\sB");
}

BetaRecord::BetaRecord(size_t& idx,
                       const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_nid(line.substr(0,5));
  energy = Energy(parse_val_uncert(line.substr(9,10),
                                   line.substr(19,2)));
  intensity = parse_norm_value(line.substr(21,8),
                               line.substr(29,2));
  LOGFT = parse_norm_value(line.substr(41,8),
                           line.substr(49,6));

  comment_flag = boost::trim_copy(line.substr(76,1));
  uniquness = boost::trim_copy(line.substr(77,2));
  quality = boost::trim_copy(line.substr(79,1));

  while ((idx+1 < data.size()) &&
         (match_cont(data[idx+1], "B") ||
          CommentsRecord::match(data[idx+1], "B")))
  {
    ++idx;
    if (CommentsRecord::match(data[idx]))
      comments.push_back(CommentsRecord(idx, data));
    else
      continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }
}

bool BetaRecord::valid() const
{
  return nuclide.valid();
}

std::string BetaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " BETA  ";
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
