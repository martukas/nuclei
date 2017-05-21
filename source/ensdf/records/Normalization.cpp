#include "Normalization.h"
#include "Fields.h"

bool NormalizationRecord::match(const std::string& line)
{
  return match_first(line, "\\sN");
}

NormalizationRecord::NormalizationRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));

  NR = parse_norm(line.substr(9,10), line.substr(19,2));
  NT = parse_norm(line.substr(21,8), line.substr(29,2));
  BR = parse_norm(line.substr(31,8), line.substr(39,2));
  NB = parse_norm(line.substr(41,8), line.substr(49,6));
  NP = parse_norm(line.substr(55,7), line.substr(62,2));

  while (i.has_more() &&
         CommentsRecord::match(i.look_ahead(), "N"))
      comments.push_back(CommentsRecord(++i));
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
