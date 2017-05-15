#pragma once

#include "ensdf_records.h"

struct NormalizationRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z]{5}\\s{2}N.*$");
  }

  static NormalizationRecord parse(size_t& idx,
                                   const std::vector<std::string>& data);

  std::string debug() const;


  NuclideId nuclide;
  UncertainDouble NR, NT, BR, NB, NP;
};

