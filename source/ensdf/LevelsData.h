#pragma once

#include "Header.h"
#include "Parent.h"

#include "History.h"
#include "QValue.h"
#include "LevelRec.h"
#include "ProductionNorm.h"

#include "ReactionInfo.h"
#include "DecayInfo.h"

struct LevelData
{
  LevelData() {}
  LevelData(ENSDFData& i);

  bool valid() const;
  std::string debug() const;

  IdRecord id;
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::map<std::string, std::string> xrefs;
  std::list<QValueRecord> qvals;
  ProdNormalizationRecord pnorm;

  std::list<GammaRecord> unplaced_gammas;

  std::list<LevelRecord> levels;

  std::list<LevelRecord> nearest_levels(const Energy& to,
                                        std::string dsid = "",
                                        double maxdif = kDoubleNaN,
                                        double zero_thresh = 0.1) const;

private:
  void read_hist(ENSDFData& i);
  void read_prelims(ENSDFData& i);
  void read_comments(ENSDFData& i);
  void read_unplaced(ENSDFData& i);
};

