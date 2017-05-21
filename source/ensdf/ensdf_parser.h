#pragma once

#include "ensdf_records.h"
#include "ensdf_basic_decay.h"

#include "comment_record.h"
#include "history_record.h"

class DaughterParser
{
public:
  DaughterParser() {}
  DaughterParser(uint16_t A, std::string directory);

  std::list<NuclideId> daughters() const;
  std::list<std::string> decays(NuclideId daughter) const;

  DecayScheme get_decay(NuclideId daughter, std::string decay_name) const;

private:
  std::list<HistoryRecord> mass_history_;
  std::list<CommentsRecord> mass_comments_;
  std::map<std::string, std::string> references_;
  std::map<NuclideId, NuclideData> nuclide_data_;

  std::list<BlockIndices> find_blocks(const std::vector<std::string> &lines) const;
  void parse(const std::vector<std::string>& lines);

  void parse_comments_block(ENSDFData &i,
                            std::list<HistoryRecord>& hist,
                            std::list<CommentsRecord>& comm);

  void parse_reference_block(ENSDFData &i);

  void modify_level(Level& currentLevel, const LevelRecord &l) const;
};


class ENSDFParser
{
public:
  ENSDFParser();
  ENSDFParser(std::string directory);

  bool good() const;

  std::list<uint16_t> masses() const;
  std::string directory() const;

  DaughterParser get_dp(uint16_t a);

private:
  std::string directory_;
  std::set<uint16_t> masses_;

  std::map<uint16_t, DaughterParser> cache_;
};
