#pragma once

#include "id_record.h"
#include "parent_record.h"
#include "DecayScheme.h"

#include "ensdf_reaction_data.h"

#include "id_record.h"
#include "history_record.h"
#include "comment_record.h"
#include "qvalue_record.h"
#include "level_record.h"
#include "gamma_record.h"
#include "prod_normalization_record.h"
#include "normalization_record.h"

struct LevelData
{
  LevelData() {}
  LevelData(const std::vector<std::string>& data,
            BlockIndices idx);

  bool valid() const;
  std::string debug() const;

  //deprecate
  BlockIndices block;

  IdRecord id;
  std::list<HistoryRecord> history;

  std::list<CommentsRecord> comments;
  std::map<std::string, std::string> xrefs;
  std::list<QValueRecord> qvals;
  ProdNormalizationRecord pnorm;

  //unplaced
  std::list<GammaRecord> gammas;

  std::list<LevelRecord> levels;

  std::list<LevelRecord> find_nearest(const Energy& to);

  void read_hist(const std::vector<std::string>& data,
                 BlockIndices& idx);

  void read_prelims(const std::vector<std::string>& data,
                    BlockIndices& idx);

  void read_comments(const std::vector<std::string>& data,
                     BlockIndices& idx);

  void read_unplaced(const std::vector<std::string>& data,
                     BlockIndices& idx);
};

struct DecayData
{
  DecayData() {}
  DecayData(const std::vector<std::string>& data,
                 BlockIndices idx);

  bool valid() const;
  std::string to_string() const;
  std::string name() const;

  //private:
  std::string parent_string() const;
  std::string halflife_string() const;

  //deprecate
  BlockIndices block;

  IdRecord id;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

  std::list<HistoryRecord> history;

  std::vector<ParentRecord> parents;
  std::vector<NormalizationRecord> norm;

  std::list<CommentsRecord> comments;
  std::list<QValueRecord> qvals;
  ProdNormalizationRecord pnorm;

  //unplaced
  std::list<AlphaRecord> alphas;
  std::list<BetaRecord> betas;
  std::list<GammaRecord> gammas;
  std::list<ParticleRecord> particles;

  std::list<LevelRecord> levels;

  void read_hist(const std::vector<std::string>& data,
                 BlockIndices& idx);

  void read_prelims(const std::vector<std::string>& data,
                    BlockIndices& idx);

  void read_unplaced(const std::vector<std::string>& data,
                     BlockIndices& idx);
};

struct NuclideData
{
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  LevelData adopted_levels;
  std::map<std::string, DecayData> decays;

  std::string add_decay(const DecayData& dec);
};
