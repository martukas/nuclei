#pragma once

#include "ensdf_records.h"

struct ReferenceRecord
{
  static bool is(const std::string& line);
  static ReferenceRecord parse(size_t& idx,
                               const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId  nuclide;
  std::string keynum, reference;
};
