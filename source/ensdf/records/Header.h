#pragma once

#include <ensdf/records/Comments.h>

enum class RecordType : int
{
  Invalid             = 0,
  AdoptedLevels       = 1 << 1,
  References          = 1 << 2,
  Comments            = 1 << 3,
  ReactionDecay       = 1 << 4,
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
  IdRecord() {}
  IdRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  std::string dsid;
  std::string extended_dsid;
  std::string dsref;
  std::string pub;
  uint16_t year;
  uint16_t month;
  RecordType type {RecordType::Invalid};

  std::list<CommentsRecord> comments;

  static RecordType is_type(std::string s);
  static std::string type_to_str(RecordType t);
};
