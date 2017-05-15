#pragma once

#include "ensdf_records.h"

struct ParentRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}\\s{2}P.*$");
  }

  static ParentRecord parse(size_t& idx,
                            const std::vector<std::string>& data);

  std::string debug() const;

  //Deprecate!!!
  static ParentRecord from_ensdf(const std::string &line);

  NuclideId nuclide;
  Energy energy;
  HalfLife hl;
  SpinParity spin;
  Energy QP;
  std::string ionization;
};
