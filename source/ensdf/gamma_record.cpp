#include "gamma_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

bool GammaRecord::match(const std::string& line)
{
  return match_first(line, "\\sG");
}

GammaRecord::GammaRecord(size_t& idx,
                         const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_nid(line.substr(0,5));
  energy = Energy(parse_val_uncert(line.substr(9,10),
                                   line.substr(19,2)));
  intensity_rel_photons = parse_norm_value(line.substr(21,8),
                                           line.substr(29,2));
  multipolarity = boost::trim_copy(line.substr(31,10));
  mixing_ratio = parse_norm_value(line.substr(41,8),
                                      line.substr(49,6));
  conversion_coef = parse_norm_value(line.substr(55,7),
                                         line.substr(62,2));
  intensity_total_transition = parse_norm_value(line.substr(64,9),
                                         line.substr(74,2));
  comment_flag = boost::trim_copy(line.substr(76,1));
  coincidence = boost::trim_copy(line.substr(77,1));
  quality = boost::trim_copy(line.substr(79,1));

  while ((idx+1 < data.size()) && (
           match_cont(data[idx+1], "\\sG")
           || CommentsRecord::match(data[idx+1], "G")
           ))
  {
    ++idx;
    if (CommentsRecord::match(data[idx], "G"))
      comments.push_back(CommentsRecord(idx, data));
    else
      continuation += "$" + boost::trim_copy(data[idx].substr(9,71));
  }
}

bool GammaRecord::valid() const
{
  return nuclide.valid() && energy.valid();
}

std::string GammaRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + " GAMMA ";
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
    ret += "\n        Continuation:" + continuation;
  for (auto c : comments)
    ret += "\n        Comment: " + c.debug();
  return ret;
}
