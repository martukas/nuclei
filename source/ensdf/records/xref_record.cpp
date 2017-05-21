#include "xref_record.h"
#include "Fields.h"
#include <boost/algorithm/string.hpp>

bool XRefRecord::match(const std::string& line)
{
  return match_first(line, "\\sX");
}

XRefRecord::XRefRecord(ENSDFData& i)
{
  const auto& line = i.read();
  if (!match(line))
    return;

  nuclide = parse_check_nid(line.substr(0, 5));
  dssym = line.substr(8, 1);
  dsid = boost::trim_copy(line.substr(9, 40));
}

std::string XRefRecord::debug() const
{
  return nuclide.symbolicName() + " XREF "
      + dssym + " = " + dsid;
}

bool XRefRecord::valid() const
{
  return nuclide.valid() && !dssym.empty() && !dsid.empty();
}
