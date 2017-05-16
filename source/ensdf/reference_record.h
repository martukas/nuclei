#pragma once

#include "ensdf_records.h"

struct ReferenceRecord
{
  ReferenceRecord() {}
  ReferenceRecord (size_t& idx,
                   const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  std::string keynum, reference;
};
