#pragma once

#include "id_record.h"
#include "parent_record.h"
#include "DecayScheme.h"

#include "id_record.h"
#include "history_record.h"
#include "comment_record.h"
#include "qvalue_record.h"
#include "level_record.h"
#include "gamma_record.h"
#include "prod_normalization_record.h"

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

struct LevelData
{
  LevelData();
  LevelData(const std::vector<std::string>& data,
            BlockIndices block_idx);

  std::string debug() const;
  bool valid() const;

  IdRecord id;
  std::list<HistoryRecord> history;
  std::list<CommentsRecord> comments;
  std::map<std::string, std::string> xrefs;
  std::list<QValueRecord> qvals;
  std::list<LevelRecord> levels;

  std::list<GammaRecord> gammas;

  ProdNormalizationRecord pnorm;

  void read_comments(const std::vector<std::string>& data,
                     BlockIndices& idx);

  void read_prelims(const std::vector<std::string>& data,
                    BlockIndices& idx);

  void read_unplaced_gammas(const std::vector<std::string>& data,
                            BlockIndices& idx);
  void read_hist(const std::vector<std::string>& data,
                 BlockIndices& idx);
};
