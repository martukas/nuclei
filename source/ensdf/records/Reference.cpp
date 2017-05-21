#include "Reference.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

bool ReferenceRecord::match(const std::string& line)
{
  return match_first(line, "\\sR");
}

ReferenceRecord::ReferenceRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_nid(line.substr(0,5));
  keynum = boost::trim_copy(line.substr(9,8));
  reference = boost::trim_copy(line.substr(17,63));
}

bool ReferenceRecord::valid() const
{
  return nuclide.valid() && !keynum.empty() && !reference.empty();
}

std::string ReferenceRecord::debug() const
{
  return nuclide.symbolicName() + " REFERENCE "
      + keynum + " = " + reference;
}
