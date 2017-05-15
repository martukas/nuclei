#pragma once

#include "ensdf_records.h"
#include "comment_record.h"

struct LevelRecord
{
  static bool is(const std::string& line);
  static LevelRecord parse(size_t& idx,
                           const std::vector<std::string>& data);

  std::string debug() const;

  NuclideId  nuclide;
  Energy     energy;
  SpinParity spin_parity;
  HalfLife   halflife;
  uint16_t   isomeric {0};
  std::string L;     //anglular momentum
  UncertainDouble S; //spectroscopic strength
  std::string comment_flag, quality;

  std::string continuation;

  std::list<CommentsRecord> comments;
};
