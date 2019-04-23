#pragma once

#include <ensdf/records/Comments.h>
#include <ensdf/records/Continuation.h>

struct ECRecord
{
  ECRecord() {}
  ECRecord(ENSDFData& i);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  Uncert intensity_beta_plus;
  Uncert intensity_ec;
  Uncert LOGFT;
  Uncert intensity_total;
  std::string comment_flag, uniquness, quality;

  std::map<std::string, Continuation> continuations_;
  std::list<CommentsRecord> comments;
};
