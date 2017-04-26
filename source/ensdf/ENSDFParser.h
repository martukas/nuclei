#pragma once

#include "ensdf_records.h"

class ENSDFParser
{
public:
  static const std::list<uint16_t> &aValues(std::string directory);

  explicit ENSDFParser(uint16_t A, std::string directory);
  uint16_t aValue() const;

  const std::list<NuclideId> daughterNuclides() const;
  const std::list< std::pair<std::string, NuclideId> > decays(const NuclideId &daughterNuclide) const;

  std::set<std::string> unknown_decays;

  DecayScheme decay(const NuclideId &daughterNuclide, const std::string &decayName) const;
private:

  struct StringSubList {
    StringSubList() : first(0), last(0) {}
    StringSubList(size_t first, size_t last) : first(first), last(last) {}
    void clear() { first=0; last=first; }
    size_t first;
    size_t last;
  };

  static std::list<uint16_t> aList;  //singleton

  static UncertainDouble parseEnsdfMixing(const std::string &s, const std::string &multipolarity);

  template <typename T> Energy findNearest(const std::map<Energy, T> &map, const Energy &val, Energy *foundVal = 0) const;
  void insertAdoptedLevelsBlock(std::map<Energy, StringSubList> *adoptblocks, const StringSubList &newblock, char dssym) const;
  std::vector<std::string> extractContinuationRecords(const StringSubList &adoptedblock, const std::list<std::string> &requestedRecords, char typeOfContinuedRecord = 'L') const;

  void parseBlocks();
  IdentificationRecord parse_header(size_t idx);


  const uint16_t a;

  std::vector<std::string> contents;
  std::map<NuclideId, BlockIndices> m_adoptedlevels; // daughter coordignates
  std::map<NuclideId, std::map<std::string, BasicDecayData > > m_decays; // daughter coordinates: (decay name: basic data)
  mutable std::map<NuclideId, std::list<std::string>> m_decayNames; // daughter coordinates: decay

  static double norm(std::string rec, double def_value);
};
