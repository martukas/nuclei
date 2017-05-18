#include "normalization_record.h"
#include "ensdf_types.h"

bool NormalizationRecord::match(const std::string& line)
{
  return match_first(line, "\\sN");
}

NormalizationRecord::NormalizationRecord(size_t& idx,
                                         const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_nid(line.substr(0,5));

  NR = parse_norm_value(line.substr(9,10), line.substr(19,2));
  NT = parse_norm_value(line.substr(21,8), line.substr(29,2));
  BR = parse_norm_value(line.substr(31,8), line.substr(39,2));
  NB = parse_norm_value(line.substr(41,8), line.substr(49,6));
  NP = parse_norm_value(line.substr(55,7), line.substr(62,2));

  while ((idx+1 < data.size()) &&
         CommentsRecord::match(data[idx+1], "N"))
    comments.push_back(CommentsRecord(++idx, data));
}

bool NormalizationRecord::valid() const
{
  return nuclide.valid();
}

std::string NormalizationRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName() + "  NORM "
      + " NR=" + NR.to_string(true)
      + " NT=" + NT.to_string(true)
      + " BR=" + BR.to_string(true)
      + " NB=" + NB.to_string(true)
      + " NP=" + NP.to_string(true);
  return ret;
}
