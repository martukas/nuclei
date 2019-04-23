#pragma once

#include <ensdf/Record.h>

struct ProdNormalizationRecord
{
  ProdNormalizationRecord() {}
  ProdNormalizationRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  Uncert NRBR, NTBR, NBBR, NP;
  bool comment_placement {false};
  int display_option {0};
  std::string caveat;
};
