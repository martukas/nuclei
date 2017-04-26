#pragma once

#include "ensdf_records.h"

class DaughterParser
{
public:
  DaughterParser();
  DaughterParser(uint16_t A, std::string directory);

  uint16_t mass_num() const;

  std::list<NuclideId> daughters() const;
  std::list< std::pair<std::string, NuclideId> > decays(NuclideId daughter) const;

  DecayScheme get_decay(NuclideId daughter, std::string decay_name) const;

  std::set<std::string> unknown_decays;

private:
  uint16_t mass_num_ {0};
  std::vector<std::string> raw_contents_;
  std::map<NuclideId, BlockIndices> adopted_levels_; // daughter coordignates
  std::map<NuclideId, std::map<std::string, BasicDecayData > > decays_; // daughter coordinates: (decay name: basic data)

  static UncertainDouble parseEnsdfMixing(const std::string &s,
                                          const std::string &multipolarity);
  template <typename T> Energy findNearest(const std::map<Energy, T> &map,
                                           const Energy &val, Energy *foundVal = 0) const;
  void insertAdoptedLevelsBlock(std::map<Energy, BlockIndices> *adoptblocks,
                                const BlockIndices &newblock, char dssym) const;
  std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
                                                      const std::list<std::string> &requestedRecords,
                                                      char typeOfContinuedRecord = 'L') const;

  static double norm(std::string rec, double def_value);
  void parseBlocks();
  IdentificationRecord parse_header(size_t idx);
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
