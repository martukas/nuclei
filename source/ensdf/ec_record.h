#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct ECRecord
{
  ECRecord() {}
  ECRecord(size_t& idx,
           const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity_beta_plus;
  UncertainDouble intensity_ec;
  UncertainDouble LOGFT;
  UncertainDouble intensity_total;
  std::string comment_flag, uniquness, quality;

  std::map<std::string, std::string> continuations_;
  std::list<CommentsRecord> comments;
};
