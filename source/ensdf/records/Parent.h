#pragma once

#include <ensdf/Record.h>

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
  SpinSet spins;
  Energy QP;
  std::string ionization;
};
