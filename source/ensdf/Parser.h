#pragma once

#include "NuclideData.h"

class DaughterParser
{
public:
  DaughterParser() {}
  DaughterParser(uint16_t A, std::string directory);

  std::list<NuclideId> daughters() const;
  std::list<std::string> decays(NuclideId daughter) const;

  DecayScheme mass_info() const;

  DecayScheme nuclide_info(NuclideId daughter,
                           double max_level_dif = 0.04) const;

  DecayScheme decay(NuclideId daughter,
                    std::string decay_name, bool merge_adopted,
                    double max_level_dif = 0.04) const;

private:
  std::list<HistoryRecord> mass_history_;
  std::list<CommentsRecord> mass_comments_;
  std::map<std::string, std::string> references_;
  std::map<NuclideId, NuclideData> nuclide_data_;

  // block parsing
  std::list<BlockIndices> find_blocks(const std::vector<std::string> &lines) const;
  void parse(const std::vector<std::string>& lines);
  void parse_reference_block(ENSDFData &i);
  void parse_comments_block(ENSDFData &i,
                            std::list<HistoryRecord>& hist,
                            std::list<CommentsRecord>& comm);



  // object construction

  static Uncert feed_norm(const ProdNormalizationRecord& pnorm,
                          std::vector<NormalizationRecord> norm);

  static Uncert gamma_norm(const ProdNormalizationRecord& pnorm,
                           std::vector<NormalizationRecord> norm);

  static Level construct_level(const LevelRecord& record,
                               Uncert intensity_norm);
  static Transition construct_transition(const GammaRecord& record,
                                         Uncert intensity_norm);

  static Nuclide construct_parent(const std::vector<ParentRecord>& parents);

  void add_text(DecayScheme& scheme,
                const std::list<HistoryRecord>& hist,
                const std::list<CommentsRecord>& comm) const;
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
