#ifndef ENSDF_RECORDS_H
#define ENSDF_RECORDS_H

#include "DecayScheme.h"
#include <list>

typedef std::pair<size_t, size_t> BlockIndices; // [startidx, size]

enum class RecordType : uint64_t
{
  Invalid           = 0,
  AdoptedLevels     = 1 << 1,
  Gammas            = 1 << 3,
  CoulombExcitation = 1 << 4,
  Decay             = 1 << 5,
  References        = 1 << 6,
  Comments          = 1 << 7,
  Tentative         = 1 << 8
};


inline constexpr RecordType operator| (RecordType a, RecordType b) {
  return static_cast<RecordType> (static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

inline constexpr RecordType operator& (RecordType a, RecordType b) {
  return static_cast<RecordType> (static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

inline RecordType operator|= (RecordType& a, RecordType b)
{
  a = a | b;
  return a;
}

inline bool test(RecordType a)
{
  return a != RecordType::Invalid;
}


struct IdentificationRecord
{
  NuclideId nuc_id;
  std::string dsid;
  std::string extended_dsid;

  RecordType type {RecordType::Invalid};

  bool continued {false};
  uint16_t numlines{0};

  static IdentificationRecord parse(const std::string& line);
  void merge_continued(IdentificationRecord other);


  std::string debug() const;

  static RecordType is_type(std::string s);
  static std::string type_to_str(RecordType t);
};

struct ParentRecord
{
  NuclideId nuclide;
  Energy energy;
  HalfLife hl;
  SpinParity spin;

  static ParentRecord from_ensdf(const std::string &precstr);
  std::string to_string() const;
};

struct BasicDecayData
{
  std::vector<ParentRecord> parents;
  NuclideId daughter;
  BlockIndices block;
  std::string dsid;

  DecayMode mode;

  HalfLife hl;
  NuclideId parent;

  static BasicDecayData from_id(const IdentificationRecord &record, BlockIndices block);

  static BasicDecayData from_ensdf(const std::string &header, BlockIndices block);
  std::string to_string() const;

private:
  static DecayMode::DecayType parseDecayType(const std::string &tstring);
};


#endif
