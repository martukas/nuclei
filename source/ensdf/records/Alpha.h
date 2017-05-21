#pragma once

#include "Comments.h"

struct AlphaRecord
{
  AlphaRecord() {}
  AlphaRecord(ENSDFData& i);
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

