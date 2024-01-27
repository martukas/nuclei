#include <ensdf/records/Gamma.h>
#include <ensdf/Fields.h>
#include <boost/algorithm/string.hpp>
#include <util/logger.h>

bool GammaRecord::match(const std::string& line)
{
  return match_first(line, "\\sG");
}

GammaRecord::GammaRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  energy = parse_energy(line.substr(9,10), line.substr(19,2));
  intensity_rel_photons = parse_norm(line.substr(21,8), line.substr(29,2));
  multipolarity = boost::trim_copy(line.substr(31,10));

  auto mixing = parse_val_uncert(line.substr(41,8), line.substr(49,6));
  mixing_ratio = eval_mixing_ratio(mixing, multipolarity);

  conversion_coef = parse_norm(line.substr(55,7), line.substr(62,2));
  intensity_total_transition = parse_norm(line.substr(64,9), line.substr(74,2));
  comment_flag = boost::trim_copy(line.substr(76,1));
  coincidence = boost::trim_copy(line.substr(77,1));
  quality = boost::trim_copy(line.substr(79,1));

  std::string continuation;
  while (i.has_more())
  {
    auto line2 = i.look_ahead();
    if (CommentsRecord::match(line2, "G"))
      comments.push_back(CommentsRecord(++i));
    else if (match_cont(line2, "\\sG"))
      continuation += "$" + boost::trim_copy(i.read_pop().substr(9,71));
    else
      break;
  }
  if (!continuation.empty())
    continuations_ = parse_continuation(continuation);
}

void GammaRecord::merge_adopted(const GammaRecord& other)
{
  if (!intensity_rel_photons.hasFiniteValue())
    intensity_rel_photons = other.intensity_rel_photons;

  if (!intensity_total_transition.hasFiniteValue())
    intensity_total_transition = other.intensity_total_transition;

  if (!conversion_coef.hasFiniteValue())
    conversion_coef = other.conversion_coef;

  if (multipolarity.empty())
    multipolarity = other.multipolarity;

  if (mixing_ratio.sign() != Uncert::SignMagnitudeDefined)
  {
    Uncert adptdelta
        = eval_mixing_ratio(other.mixing_ratio, multipolarity);
    if (adptdelta.sign() > mixing_ratio.sign())
      mixing_ratio = adptdelta;
  }

  merge_continuations(continuations_,
                      other.continuations_);
                      // "<Gamma>(" + nuclide.symbolicName()
                      // + ":" + energy.to_string() + ")");

  for (const auto& com : other.comments)
    comments.push_back(com);
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
  for (auto c : continuations_)
    ret += "\n        Continuation: " + c.first
        + " = " + c.second.value_refs();
  for (auto c : comments)
    ret += "\n        Comment: " + c.debug();
  return ret;
}
