#pragma once

#include "LevelsData.h"

struct AdoptedLevels : public LevelsData
{
  AdoptedLevels() {}
  AdoptedLevels(ENSDFData& i);

  bool valid() const;
  std::string debug() const;

  std::list<LevelRecord> nearest_levels(const Energy& to,
                                        std::string dsid = "",
                                        double maxdif = kDoubleNaN,
                                        double zero_thresh = 0.1) const;

};

