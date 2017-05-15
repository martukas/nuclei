#pragma once

#include "ensdf_records.h"

enum class RecordType : uint64_t
{
  Invalid             = 0,
  AdoptedLevels       = 1 << 1,
  Gammas              = 1 << 3,
  CoulombExcitation   = 1 << 4,
  Decay               = 1 << 5,
  References          = 1 << 6,
  Comments            = 1 << 7,
  MuonicAtom          = 1 << 8,
  InelasticScattering = 1 << 9,
  HiXng               = 1 << 10,
  Tentative           = 1 << 11,
  Reaction            = 1 << 12
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

struct IdRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line, "^[\\s0-9A-Za-z]{6}\\s{3}.*$");
  }

  static IdRecord parse(size_t& idx,
                        const std::vector<std::string>& data);

  std::string debug() const;
  void reflect_parse() const;

  NuclideId nuc_id;
  std::string dsid;
  std::string extended_dsid;
  std::string dsref;
  std::string pub;
  uint16_t year;
  uint16_t month;
  RecordType type {RecordType::Invalid};

  static RecordType is_type(std::string s);
  static std::string type_to_str(RecordType t);
};
