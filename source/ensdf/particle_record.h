#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct ParticleRecord
{
  ParticleRecord() {}
  ParticleRecord(size_t& idx,
                 const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

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
