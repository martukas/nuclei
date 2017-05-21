#pragma once

#include "Record.h"
#include "comment_record.h"

struct NormalizationRecord
{
  NormalizationRecord() {}
  NormalizationRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  UncertainDouble NR, NT, BR, NB, NP;

  std::list<CommentsRecord> comments;
};

