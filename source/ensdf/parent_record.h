#pragma once

#include "ensdf_records.h"

struct ParentRecord
{
  ParentRecord() {}
  ParentRecord(size_t& idx,
               const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  Energy energy;
  HalfLife hl;
  SpinParity spin;
  Energy QP;
  std::string ionization;
};
