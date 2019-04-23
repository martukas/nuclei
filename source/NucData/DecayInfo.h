#pragma once

#include <NucData/nid.h>
#include "DecayMode.h"
#include <NucData/HalfLife.h>

struct DecayInfo
{
  DecayInfo() {}

  bool valid() const;
  std::string to_string() const;
  std::string name() const;

  NuclideId parent;
  DecayMode mode;
  HalfLife hl;
};
