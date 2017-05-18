#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct NormalizationRecord
{
  NormalizationRecord() {}
  NormalizationRecord(size_t& idx,
                      const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  UncertainDouble NR, NT, BR, NB, NP;

  std::list<CommentsRecord> comments;
};

