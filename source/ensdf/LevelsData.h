#pragma once

#include <ensdf/records/Header.h>
#include <ensdf/records/History.h>
#include <ensdf/records/QValue.h>
#include <ensdf/records/LevelRec.h>

#include <ensdf/records/Parent.h>

#include <ensdf/records/Normalization.h>
#include <ensdf/records/ProductionNorm.h>

#include <NucData/ReactionInfo.h>
#include <NucData/DecayInfo.h>

struct LevelsData
{
  LevelsData() {}
  LevelsData(ENSDFData& i);

  IdRecord id;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;
  bool adopted {false};
  bool gammas {false};

  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;

  std::list<QValueRecord> qvals;

  ProdNormalizationRecord pnorm;
  std::vector<NormalizationRecord> norm;

  Transitions unplaced;
  std::list<LevelRecord> levels;

  //for adopted levels only
  std::map<std::string, std::string> xrefs;

  //for decays only
  std::vector<ParentRecord> parents;

  std::string name() const;
  std::string debug() const;
  std::list<LevelRecord> nearest_levels(const Energy& to,
                                        std::string dsid = "",
                                        double maxdif = kDoubleNaN,
                                        double zero_thresh = 0.1) const;

protected:
  void read_hist(ENSDFData& i);
  void read_prelims(ENSDFData& i);
  void read_comments(ENSDFData& i);
  void read_unplaced(ENSDFData& i);
  void read_levels(ENSDFData& i);

  std::string parent_string() const;
  std::string halflife_string() const;
};
