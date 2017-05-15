#include "normalization_record.h"
#include "ensdf_types.h"

NormalizationRecord NormalizationRecord::parse(size_t& idx,
                                               const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return NormalizationRecord();
  auto line = data[idx];

  NormalizationRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));

  ret.NR = parse_norm_value(line.substr(9,10), line.substr(19,2));
  ret.NT = parse_norm_value(line.substr(21,8), line.substr(29,2));
  ret.BR = parse_norm_value(line.substr(31,8), line.substr(39,2));
  ret.NB = parse_norm_value(line.substr(41,8), line.substr(49,6));
  ret.NP = parse_norm_value(line.substr(55,7), line.substr(62,2));

  return ret;
}

std::string NormalizationRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName()
      + " NR=" + NR.to_string(true)
      + " NT=" + NT.to_string(true)
      + " BR=" + BR.to_string(true)
      + " NB=" + NB.to_string(true)
      + " NP=" + NP.to_string(true);
  return ret;
}
