#pragma once

#include "ensdf_records.h"
#include "comment_record.h"
#include "gamma_record.h"

struct LevelRecord
{
  LevelRecord() {}
  LevelRecord(size_t& idx,
              const std::vector<std::string>& data);

  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId  nuclide;
  Energy     energy;
  SpinParity spin_parity;
  HalfLife   halflife;
  uint16_t   isomeric {0};
  std::string L;     //anglular momentum
  UncertainDouble S; //spectroscopic strength
  std::string comment_flag, quality;

  std::string continuation;

  std::string offset;

  std::list<CommentsRecord> comments;
  std::list<GammaRecord> gammas;
};
