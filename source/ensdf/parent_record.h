#pragma once

#include "ensdf_records.h"

struct ParentRecord
{
  ParentRecord() {}
  ParentRecord(ENSDFData& i);
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
