#include "particle_record.h"
#include "ensdf_types.h"
#include <boost/regex.hpp>
#include "qpx_util.h"

bool ParticleRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Za-z]{5}\\s\\s[\\sD][NPA].*$");
}

ParticleRecord
ParticleRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return ParticleRecord();
  auto line = data[idx];

  ParticleRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.delayed = (line[7] == 'D');
  ret.particle = line.substr(8,1);

  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.intensity = parse_norm_value(line.substr(21,8),
                                   line.substr(29,2));
  ret.energy_intermediate = boost::trim_copy(line.substr(31,8));
  ret.transition_width = parse_norm_value(line.substr(39,10),
                                          line.substr(49,6));
  ret.L = boost::trim_copy(line.substr(55,9));

  ret.comment_flag = boost::trim_copy(line.substr(76,1));
  ret.coincidence = boost::trim_copy(line.substr(77,1));
  ret.quality = boost::trim_copy(line.substr(79,1));

  boost::regex filter("^[\\s0-9A-Za-z]{5}[02-9A-Za-z].[\\sD][NPA].*$");
  while ((idx+1 < data.size()) &&
         (boost::regex_match(data[idx+1], filter) ||
          CommentsRecord::match(data[idx+1])))
  {
    ++idx;
    if (CommentsRecord::match(data[idx]))
      ret.comments.push_back(CommentsRecord(idx, data));
    else
      ret.continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }

  return ret;
}

std::string ParticleRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
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
  if (!continuation.empty())
    ret += "\n  Continuation:" + continuation;
  for (auto c : comments)
    ret += "\n  Comment: " + c.debug();
  return ret;
}
