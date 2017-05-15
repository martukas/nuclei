#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct BetaRecord
{
  static bool is(const std::string& line);

  static BetaRecord parse(size_t& idx,
                           const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity;
  UncertainDouble LOGFT;
  std::string comment_flag, uniquness, quality;

  std::string continuation;
  std::list<CommentsRecord> comments;
};
