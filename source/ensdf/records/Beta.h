#pragma once

#include "Comments.h"

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

  std::map<std::string, std::string> continuations_;
  std::list<CommentsRecord> comments;
};
