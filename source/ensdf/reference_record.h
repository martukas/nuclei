#pragma once

#include "ensdf_records.h"

struct ReferenceRecord
{
  ReferenceRecord() {}
  ReferenceRecord (ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  std::string keynum, reference;
};
