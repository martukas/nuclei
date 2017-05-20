#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct AlphaRecord
{
  AlphaRecord() {}
  AlphaRecord(size_t& idx,
              const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity_alpha;
  UncertainDouble hindrance_factor;
  std::string comment_flag, quality;

  std::list<CommentsRecord> comments;
  std::map<std::string, std::string> continuations_;
};

