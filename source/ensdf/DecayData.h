#pragma once

#include "id_record.h"
#include "parent_record.h"
#include "DecayScheme.h"

#include "ReactionInfo.h"
#include "DecayInfo.h"

#include "id_record.h"
#include "history_record.h"
#include "comment_record.h"
#include "qvalue_record.h"
#include "level_record.h"
#include "gamma_record.h"
#include "prod_normalization_record.h"
#include "normalization_record.h"

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
