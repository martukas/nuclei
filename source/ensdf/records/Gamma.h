#pragma once

#include <ensdf/records/Comments.h>
#include <ensdf/records/Continuation.h>

struct GammaRecord
{
  GammaRecord() {}
  GammaRecord(ENSDFData& i);
  static bool match(const std::string& line);
  void merge_adopted(const GammaRecord& other);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  Uncert intensity_rel_photons;
  Uncert intensity_total_transition;
  Uncert mixing_ratio;
  Uncert conversion_coef;
  std::string multipolarity;
  std::string comment_flag, coincidence, quality;

  std::map<std::string, Continuation> continuations_;
  std::list<CommentsRecord> comments;
};
