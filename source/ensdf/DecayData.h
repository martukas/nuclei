#pragma once

#include "LevelsData.h"

#include "ReactionInfo.h"
#include "DecayInfo.h"


struct DecayData : public LevelsData
{
  DecayData() {}
  DecayData(ENSDFData& i);

  bool valid() const;
  std::string name() const;
  std::string debug() const;

  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

private:
  std::string parent_string() const;
  std::string halflife_string() const;
};
