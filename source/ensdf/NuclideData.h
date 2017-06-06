#pragma once

#include "AdoptedLevels.h"
#include "DecayData.h"

struct NuclideData
{
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::list<AdoptedLevels> adopted_levels;
  std::map<std::string, DecayData> decays;

  void add_adopted(const AdoptedLevels& dec);
  std::string add_decay(const DecayData& dec);
  void merge_adopted(DecayData& decaydata,
                     double max_level_dif = 0.04,
                     double max_gamma_dif = 0.005) const;
};
