#include "Alpha.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

bool AlphaRecord::match(const std::string& line)
{
  return match_first(line, "\\sA");
}

AlphaRecord::AlphaRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  intensity_alpha = parse_norm(line.substr(21,8), line.substr(29,2));
  hindrance_factor = parse_norm(line.substr(31,8), line.substr(39,2));
  comment_flag = boost::trim_copy(line.substr(76,1));
  quality = boost::trim_copy(line.substr(79,1));

  std::string continuation;
  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (CommentsRecord::match(line2, "A"))
      comments.push_back(CommentsRecord(++i));
    else if (match_cont(line2, "\\sA"))
      continuation += "$" + boost::trim_copy(i.read_pop().substr(9,71));
    else
      break;
  }
  if (!continuation.empty())
    continuations_ = parse_continuation(continuation);
}

bool AlphaRecord::valid() const
{
  return nuclide.valid();
}

std::string AlphaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " ALPHA ";
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (intensity_alpha.hasFiniteValue())
    ret += " Intensity(A)=" + intensity_alpha.to_string(true);
  if (hindrance_factor.hasFiniteValue())
    ret += " HindranceFactor=" + hindrance_factor.to_string(true);
  if (!comment_flag.empty())
    ret += " comment=" + comment_flag;
  if (!quality.empty())
    ret += " quality=" + quality;
  for (auto c : continuations_)
    ret += "\n      Continuation: " + c.first + " = " + c.second;
  for (auto c : comments)
    ret += "\n      Comment: " + c.debug();
  return ret;
}
