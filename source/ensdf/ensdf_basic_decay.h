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

struct DecayData
{
  DecayData() {}
  DecayData(const std::vector<std::string>& data,
                 BlockIndices idx);

//  bool valid() const;
  std::string to_string() const;

  //deprecate
  BlockIndices block;

  IdRecord id;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

  std::list<HistoryRecord> history;

  std::vector<ParentRecord> parents;

  std::list<CommentsRecord> comments;
  std::list<QValueRecord> qvals;
  NormalizationRecord norm;
  ProdNormalizationRecord pnorm;

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

struct LevelData
{
  LevelData();
  LevelData(const std::vector<std::string>& data,
            BlockIndices idx);

  bool valid() const;
  std::string debug() const;

  IdRecord id;
  std::list<HistoryRecord> history;

  std::list<CommentsRecord> comments;
  std::map<std::string, std::string> xrefs;
  std::list<QValueRecord> qvals;
  ProdNormalizationRecord pnorm;

  std::list<GammaRecord> gammas;

  std::list<LevelRecord> levels;

  void read_hist(const std::vector<std::string>& data,
                 BlockIndices& idx);

  void read_prelims(const std::vector<std::string>& data,
                    BlockIndices& idx);

  void read_comments(const std::vector<std::string>& data,
                     BlockIndices& idx);

  void read_unplaced_gammas(const std::vector<std::string>& data,
                            BlockIndices& idx);

};

