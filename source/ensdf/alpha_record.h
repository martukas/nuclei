#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct AlphaRecord
{
  static bool is(const std::string& line);
  static AlphaRecord parse(size_t& idx,
                           const std::vector<std::string>& data);
  std::string debug() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity_alpha;
  UncertainDouble hindrance_factor;
  std::string comment_flag, quality;

  std::list<CommentsRecord> comments;
};

