#pragma once

#include "Comments.h"
#include "Continuation.h"

struct ParticleRecord
{
  ParticleRecord() {}
  ParticleRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;

  bool delayed {false};
  std::string particle;

  Energy     energy;
  Uncert intensity;
  Uncert transition_width;
  std::string energy_intermediate, L;
  std::string comment_flag, coincidence, quality;

  std::map<std::string, Continuation> continuations_;
  std::list<CommentsRecord> comments;
};
