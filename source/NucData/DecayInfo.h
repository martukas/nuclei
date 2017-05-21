#pragma once

#include "nid.h"
#include "DecayMode.h"
#include "HalfLife.h"

struct DecayInfo
{
  DecayInfo() {}
  DecayInfo(std::string dsid);

  bool valid() const;
  std::string to_string() const;
  std::string name() const;

  NuclideId parent;
  DecayMode mode;
  HalfLife hl;
};
