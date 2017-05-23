#pragma once

#include "Record.h"
#include "Comments.h"

struct HistoryRecord
{
  HistoryRecord() {}
  HistoryRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  std::map<std::string, std::string> kvps;

private:
  void add_kvp(const std::string& key, const std::string& value);
};
