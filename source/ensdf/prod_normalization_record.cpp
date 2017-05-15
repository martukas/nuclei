#include "prod_normalization_record.h"
#include "ensdf_types.h"
#include "qpx_util.h"
#include <boost/regex.hpp>

ProdNormalizationRecord
ProdNormalizationRecord::parse(size_t& idx,
                               const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return ProdNormalizationRecord();
  auto line = data[idx];

  ProdNormalizationRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));

  ret.NRBR = parse_norm_value(line.substr(9,10), line.substr(19,2));
  ret.NTBR = parse_norm_value(line.substr(21,8), line.substr(29,2));
  ret.NBBR = parse_norm_value(line.substr(41,8), line.substr(49,6));
  ret.NP = parse_norm_value(line.substr(55,7), line.substr(62,2));
  ret.comment_placement = (line.substr(76,1) == "C");
  auto dopt = line.substr(77,1);
  if (is_number(dopt))
    ret.display_option = std::stoi(dopt);

  boost::regex filter("^[\\s0-9A-Za-z]{5}[02-9A-Za-z]PN.*$");
  while ((idx+1 < data.size()) &&
         boost::regex_match(data[idx+1], filter))
  {
    ++idx;
    ret.caveat = boost::trim_copy(data[idx].substr(9, 71));
  }

  return ret;
}

std::string ProdNormalizationRecord::debug() const
{
  std::string ret;
  ret = nuclide.symbolicName();
  if (NRBR.hasFiniteValue())
    ret += " NRBR=" + NRBR.to_string(true);
  if (NTBR.hasFiniteValue())
    ret += " NTBR=" + NTBR.to_string(true);
  if (NBBR.hasFiniteValue())
    ret += " NBBR=" + NBBR.to_string(true);
  if (NP.hasFiniteValue())
    ret += " NP=" + NP.to_string(true);
  ret += " CommentPlacement="
      + std::string(comment_placement ? "True" : "False")
      + " DisplayOption=" + std::to_string(display_option);
  if (!caveat.empty())
    ret += " Caveat=" + caveat;
  return ret;
}
