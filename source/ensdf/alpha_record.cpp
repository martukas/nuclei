#include "alpha_record.h"
#include "ensdf_types.h"
#include <boost/regex.hpp>
#include "qpx_util.h"

bool AlphaRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Z]{6}\\sA\\s.*$");
}

AlphaRecord
AlphaRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return AlphaRecord();
  auto line = data[idx];

  AlphaRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.intensity_alpha = parse_norm_value(line.substr(21,8),
                                         line.substr(29,2));
  ret.hindrance_factor = parse_norm_value(line.substr(31,8),
                                          line.substr(39,2));

  ret.comment_flag = boost::trim_copy(line.substr(76,1));
  ret.quality = boost::trim_copy(line.substr(79,1));

  while ((idx+1 < data.size()) && CommentsRecord::match(data[idx+1]))
  {
    ++idx;
    ret.comments.push_back(CommentsRecord(++idx, data));
  }

  return ret;
}

std::string AlphaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
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
  for (auto c : comments)
    ret += "\n  Comment: " + c.debug();
  return ret;
}
