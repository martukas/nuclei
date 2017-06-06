#pragma once

#include "Header.h"
#include "History.h"
#include "QValue.h"
#include "LevelRec.h"

#include "Parent.h"

#include "Normalization.h"
#include "ProductionNorm.h"

struct LevelsData
{
  LevelsData() {}
  LevelsData(ENSDFData& i);

  IdRecord id;
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::list<QValueRecord> qvals;

  // some issues here
  ProdNormalizationRecord pnorm;
  std::vector<NormalizationRecord> norm;

  //for adopted levels only
  std::map<std::string, std::string> xrefs;

  //for decays only
  std::vector<ParentRecord> parents;

  std::list<LevelRecord> levels;
  Transitions unplaced;

protected:
  void read_hist(ENSDFData& i);
  void read_prelims(ENSDFData& i);
  void read_comments(ENSDFData& i);
  void read_unplaced(ENSDFData& i);
  void read_levels(ENSDFData& i);
};
