#include "xref_record.h"
#include "ensdf_types.h"
#include <boost/algorithm/string.hpp>

XRefRecord XRefRecord::parse(size_t& idx,
                             const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return XRefRecord();

  auto line = data[idx];
  XRefRecord ret;

  ret.nuclide = parse_check_nid(line.substr(0, 5));
  ret.dssym = line.substr(8, 1);
  ret.dsid = boost::trim_copy(line.substr(9, 40));
  return ret;
}

std::string XRefRecord::debug() const
{
  return nuclide.symbolicName() + " xref "
      + " dssym=" + dssym + " dsid=" + dsid;
}
