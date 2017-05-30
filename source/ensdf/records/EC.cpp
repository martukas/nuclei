#include "EC.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

bool ECRecord::match(const std::string& line)
{
  return match_first(line, "\\sE");
}

ECRecord::ECRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  intensity_beta_plus = parse_norm(line.substr(21,8), line.substr(29,2));
  intensity_ec = parse_norm(line.substr(31,8), line.substr(39,2));
  LOGFT = parse_norm(line.substr(41,8), line.substr(49,6));
  intensity_total = parse_norm(line.substr(64,10), line.substr(74,2));
  comment_flag = boost::trim_copy(line.substr(76,1));
  uniquness = boost::trim_copy(line.substr(77,2));
  quality = boost::trim_copy(line.substr(79,1));

  std::string continuation;
  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (CommentsRecord::match(line2, "E"))
      comments.push_back(CommentsRecord(++i));
    else if (match_cont(line2, "\\sE"))
      continuation += "$" + boost::trim_copy(i.read_pop().substr(9,71));
    else
      break;
  }
  if (!continuation.empty())
    continuations_ = parse_continuation(continuation);
}

bool ECRecord::valid() const
{
  return nuclide.valid();
}

std::string ECRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " EC ";
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (intensity_beta_plus.hasFiniteValue())
    ret += " Intensity(B+)=" + intensity_beta_plus.to_string(true);
  if (intensity_ec.hasFiniteValue())
    ret += " Intensity(EC)=" + intensity_ec.to_string(true);
  if (LOGFT.hasFiniteValue())
    ret += " logFT=" + LOGFT.to_string(true);
  if (intensity_total.hasFiniteValue())
    ret += " Intensity(total)=" + intensity_total.to_string(true);
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
