#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

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
  UncertainDouble intensity_rel_photons;
  UncertainDouble intensity_total_transition;
  UncertainDouble mixing_ratio;
  UncertainDouble conversion_coef;
  std::string multipolarity;
  std::string comment_flag, coincidence, quality;

  std::map<std::string, std::string> continuations_;
  std::list<CommentsRecord> comments;
};
