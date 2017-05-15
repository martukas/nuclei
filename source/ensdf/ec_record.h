#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct ECRecord
{
  static bool is(const std::string& line);
  static ECRecord parse(size_t& idx,
                        const std::vector<std::string>& data);
  std::string debug() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity_beta_plus;
  UncertainDouble intensity_ec;
  UncertainDouble LOGFT;
  UncertainDouble intensity_total;
  std::string comment_flag, uniquness, quality;

  std::string continuation;
  std::list<CommentsRecord> comments;
};
