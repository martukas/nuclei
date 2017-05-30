#include "Beta.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

bool BetaRecord::match(const std::string& line)
{
  return match_first(line, "\\sB");
}

BetaRecord::BetaRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  intensity = parse_norm(line.substr(21,8), line.substr(29,2));
  LOGFT = parse_norm(line.substr(41,8), line.substr(49,6));
  comment_flag = boost::trim_copy(line.substr(76,1));
  uniquness = boost::trim_copy(line.substr(77,2));
  quality = boost::trim_copy(line.substr(79,1));

  std::string continuation;
  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (CommentsRecord::match(line2, "B"))
      comments.push_back(CommentsRecord(++i));
    else if (match_cont(line2, "\\sB"))
      continuation += "$" + boost::trim_copy(i.read_pop().substr(9,71));
    else
      break;
  }

  if (!continuation.empty())
    continuations_ = parse_continuation(continuation);
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
  for (auto c : continuations_)
    ret += "\n        Continuation: " + c.first
        + " = " + c.second.value_refs();
  for (auto c : comments)
    ret += "\n        Comment: " + c.debug();
  return ret;
}
