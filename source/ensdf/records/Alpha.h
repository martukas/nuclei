#pragma once

#include <ensdf/records/Comments.h>
#include <ensdf/records/Continuation.h>

struct AlphaRecord
{
  AlphaRecord() {}
  AlphaRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  Uncert intensity_alpha;
  Uncert hindrance_factor;
  std::string comment_flag, quality;

  std::list<CommentsRecord> comments;
  std::map<std::string, Continuation> continuations_;
};

