#pragma once

#include <ensdf/Record.h>

struct XRefRecord
{
  XRefRecord() {}
  XRefRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  std::string dssym;
  std::string dsid;
};
