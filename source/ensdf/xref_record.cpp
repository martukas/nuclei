#include "xref_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

bool XRefRecord::match(const std::string& line)
{
  return match_first(line, "\\sX");
}

XRefRecord::XRefRecord(size_t& idx,
                       const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !match(data[idx]))
    return;
  const auto& line = data[idx];

  nuclide = parse_check_nid(line.substr(0, 5));
  dssym = line.substr(8, 1);
  dsid = boost::trim_copy(line.substr(9, 40));
}

std::string XRefRecord::debug() const
{
  return nuclide.symbolicName() + " XREF "
      + dssym + "->" + dsid;
}

bool XRefRecord::valid() const
{
  return nuclide.valid() && !dssym.empty() && !dsid.empty();
}
