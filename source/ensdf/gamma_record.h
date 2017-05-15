#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct GammaRecord
{
  static bool is(const std::string& line);
  static GammaRecord parse(size_t& idx,
                           const std::vector<std::string>& data);
  std::string debug() const;

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
