#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct ParticleRecord
{
  static bool is(const std::string& line);
  static ParticleRecord parse(size_t& idx,
                        const std::vector<std::string>& data);
  std::string debug() const;

  NuclideId  nuclide;

  bool delayed {false};
  std::string particle;

  Energy     energy;
  UncertainDouble intensity;
  UncertainDouble transition_width;
  std::string energy_intermediate, L;
  std::string comment_flag, coincidence, quality;

  std::string continuation;
  std::list<CommentsRecord> comments;
};
