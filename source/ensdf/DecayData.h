#pragma once

#include "Header.h"
#include "Parent.h"

#include "History.h"
#include "QValue.h"
#include "LevelRec.h"
#include "ProductionNorm.h"
#include "Normalization.h"

#include "ReactionInfo.h"
#include "DecayInfo.h"


struct DecayData
{
  DecayData() {}
  DecayData(ENSDFData& i);

  bool valid() const;
  std::string name() const;
  std::string debug() const;

  IdRecord id;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::vector<ParentRecord> parents;
  std::vector<NormalizationRecord> norm;
  ProdNormalizationRecord pnorm;
  std::list<QValueRecord> qvals;

  std::list<AlphaRecord> unplaced_alphas;
  std::list<BetaRecord> unplaced_betas;
  std::list<GammaRecord> unplaced_gammas;
  std::list<ParticleRecord> unplaced_particles;

  std::list<LevelRecord> levels;

private:
  std::string parent_string() const;
  std::string halflife_string() const;

  void read_hist(ENSDFData& i);
  void read_prelims(ENSDFData& i);
  void read_unplaced(ENSDFData& i);
};
