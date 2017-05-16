#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct GammaRecord
{
  GammaRecord() {}
  GammaRecord(size_t& idx,
              const std::vector<std::string>& data);

  static bool match(const std::string& line);

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

  std::string continuation;
  std::list<CommentsRecord> comments;
};
