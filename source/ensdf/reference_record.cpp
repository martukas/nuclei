#include "reference_record.h"
#include "ensdf_types.h"
#include "qpx_util.h"

bool ReferenceRecord::is(const std::string& line)
{
  return match_record_type(line,
                           "^[\\s0-9]{3}\\s{4}R\\s.*$");
}

ReferenceRecord
ReferenceRecord::parse(size_t& idx,
                   const std::vector<std::string>& data)
{
  if ((idx >= data.size()) || !is(data[idx]))
    return ReferenceRecord();
  auto line = data[idx];

  ReferenceRecord ret;

  ret.nuclide = parse_nid(line.substr(0,5));
  ret.keynum = boost::trim_copy(line.substr(9,8));
  ret.reference = boost::trim_copy(line.substr(17,63));

  return ret;
}

std::string ReferenceRecord::debug() const
{
  return nuclide.symbolicName()
      + keynum + " = " + reference;
}
