#pragma once

#include "ensdf_records.h"

struct XRefRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}\\s{2}X.*$");
  }

  static XRefRecord parse(size_t& idx,
                          const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId nuclide;
  std::string dssym;
  std::string dsid;
};
