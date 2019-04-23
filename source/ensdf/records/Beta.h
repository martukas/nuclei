#pragma once

#include <ensdf/records/Comments.h>
#include <ensdf/records/Continuation.h>

struct BetaRecord
{
  BetaRecord() {}
  BetaRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  Uncert intensity;
  Uncert LOGFT;
  std::string comment_flag, uniquness, quality;

  std::map<std::string, Continuation> continuations_;
  std::list<CommentsRecord> comments;
};
