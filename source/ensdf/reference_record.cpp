#include "reference_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

bool ReferenceRecord::match(const std::string& line)
{
  return match_first(line, "\\sR");
}

ReferenceRecord::ReferenceRecord(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

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
