#include "particle_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

bool ParticleRecord::match(const std::string& line)
{
  return match_first(line, "\\s[\\sD][NPA]");
}

ParticleRecord::ParticleRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  delayed = (line[7] == 'D');
  particle = line.substr(8,1);

  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  intensity = parse_norm(line.substr(21,8), line.substr(29,2));
  energy_intermediate = boost::trim_copy(line.substr(31,8));
  transition_width = parse_norm(line.substr(39,10), line.substr(49,6));
  L = boost::trim_copy(line.substr(55,9));

  comment_flag = boost::trim_copy(line.substr(76,1));
  coincidence = boost::trim_copy(line.substr(77,1));
  quality = boost::trim_copy(line.substr(79,1));

  std::string continuation;
  std::string signature = line.substr(7,2);
  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (CommentsRecord::match(line2, signature))
      comments.push_back(CommentsRecord(++i));
    else if (match_cont(line2, "\\s" + signature))
      continuation += "$" + boost::trim_copy(i.read_pop().substr(9,71));
    else
      break;
  }
  if (!continuation.empty())
    continuations_ = parse_continuation(continuation);
}

bool ParticleRecord::valid() const
{
  return nuclide.valid();
}

std::string ParticleRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " PARTICLE ";
  if (delayed)
    ret += " Delayed-";
  ret += particle;
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (intensity.hasFiniteValue())
    ret += " Intensity=" + intensity.to_string(true);
  if (transition_width.hasFiniteValue())
    ret += " TransitionWidth=" + transition_width.to_string(true);
  if (!energy_intermediate.empty())
    ret += " EnergyIntermediate=" + energy_intermediate;
  if (!L.empty())
    ret += " L=" + L;
  if (!comment_flag.empty())
    ret += " comment=" + comment_flag;
  if (!coincidence.empty())
    ret += " coincidence=" + coincidence;
  if (!quality.empty())
    ret += " quality=" + quality;
  for (auto c : continuations_)
    ret += "\n      Continuation: " + c.first + " = " + c.second;
  for (auto c : comments)
    ret += "\n  Comment: " + c.debug();
  return ret;
}
