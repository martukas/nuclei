#pragma once

#include "ensdf_records.h"

struct ProdNormalizationRecord
{
  ProdNormalizationRecord() {}
  ProdNormalizationRecord(size_t& idx,
                          const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  UncertainDouble NRBR, NTBR, NBBR, NP;
  bool comment_placement {false};
  int display_option {0};
  std::string caveat;
};
