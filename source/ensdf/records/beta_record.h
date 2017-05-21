#pragma once

#include "Record.h"
#include "comment_record.h"

struct BetaRecord
{
  BetaRecord() {}
  BetaRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  UncertainDouble intensity;
  UncertainDouble LOGFT;
  std::string comment_flag, uniquness, quality;

  std::map<std::string, std::string> continuations_;
  std::list<CommentsRecord> comments;
};
