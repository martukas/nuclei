#pragma once

#include "ensdf_records.h"

struct ProdNormalizationRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z]{5}[\\s02-9A-Za-z]PN.*$");
  }

  static ProdNormalizationRecord parse(size_t& idx,
                                       const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId nuclide;
  UncertainDouble NRBR, NTBR, NBBR, NP;
  bool comment_placement {false};
  int display_option {0};
  std::string caveat;
};
