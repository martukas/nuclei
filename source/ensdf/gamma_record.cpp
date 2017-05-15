#include "gamma_record.h"
#include "ensdf_types.h"
#include <boost/regex.hpp>
#include "qpx_util.h"

bool GammaRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9A-Za-z]{5}\\s\\sG\\s.*$");
}

GammaRecord
GammaRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return GammaRecord();
  auto line = data[idx];

  GammaRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.energy = parse_energy(line.substr(9,10), line.substr(19,2));
  ret.intensity_rel_photons = parse_norm_value(line.substr(21,8),
                                               line.substr(29,2));
  ret.multipolarity = boost::trim_copy(line.substr(31,10));
  ret.mixing_ratio = parse_norm_value(line.substr(41,8),
                                      line.substr(49,6));
  ret.conversion_coef = parse_norm_value(line.substr(55,7),
                                         line.substr(62,2));
  ret.intensity_total_transition = parse_norm_value(line.substr(64,9),
                                         line.substr(74,2));
  ret.comment_flag = boost::trim_copy(line.substr(76,1));
  ret.coincidence = boost::trim_copy(line.substr(77,1));
  ret.quality = boost::trim_copy(line.substr(79,1));

  boost::regex filter("^[\\s0-9A-Za-z]{5}[02-9A-Za-z@$].G.*$");
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

std::string GammaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
  if (energy.valid())
    ret += " Energy=" + energy.to_string();
  if (intensity_rel_photons.hasFiniteValue())
    ret += " Intensity(phot)=" + intensity_rel_photons.to_string(true);
  if (intensity_total_transition.hasFiniteValue())
    ret += " Intensity(tot)=" + intensity_total_transition.to_string(true);
  if (mixing_ratio.hasFiniteValue())
    ret += " MixRatio=" + mixing_ratio.to_string(true);
  if (conversion_coef.hasFiniteValue())
    ret += " ConversionCoef=" + conversion_coef.to_string(true);
  if (!multipolarity.empty())
    ret += " mpol=" + multipolarity;
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
