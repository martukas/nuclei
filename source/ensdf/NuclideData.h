#pragma once

#include "LevelsData.h"

struct NuclideData
{
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::map<std::string, LevelsData> decays;

  std::string add(const LevelsData& dec);
  void merge_adopted(LevelsData& decaydata,
                     double max_level_dif = 0.04,
                     double max_gamma_dif = 0.005) const;
};
