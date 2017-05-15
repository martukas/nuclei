#pragma once

#include "id_record.h"
#include "parent_record.h"
#include "DecayScheme.h"

struct BasicDecayData
{
  static BasicDecayData from_id(const IdRecord &record,
                                BlockIndices block);
  std::string to_string() const;

  //general
  NuclideId daughter;
  BlockIndices block;
  std::string dsid;

  //header data
  NuclideId parent;
  DecayMode mode;
  HalfLife hl;

  //extra shit
  std::vector<ParentRecord> parents;

private:
  static DecayMode parseDecayType(const std::string &tstring);
};

