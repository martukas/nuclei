#pragma once

#include "ensdf_records.h"

struct HistoryRecord
{
  static bool is(const std::string& line);

  static HistoryRecord parse(size_t& idx,
                             const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId nuc_id;
  std::map<std::string, std::string> kvps;
};
