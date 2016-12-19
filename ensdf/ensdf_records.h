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
  MuonicAtom        = 1 << 8,
  HiXng             = 1 << 9,
  Tentative         = 1 << 10
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
  std::string dsref;
  std::string pub;
  uint16_t year;
  uint16_t month;

  RecordType type {RecordType::Invalid};

  bool continued {false};
  uint16_t numlines{1}; //deprecate

  static IdentificationRecord parse(const std::string& line);
  void merge_continued(IdentificationRecord other);


  std::string debug() const;

  static RecordType is_type(std::string s);
  static std::string type_to_str(RecordType t);
};

struct CommentsRecord
{
  static CommentsRecord from_id(const IdentificationRecord &record, BlockIndices block);

  //general
  NuclideId nuclide;
  BlockIndices block;
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
  static BasicDecayData from_id(const IdentificationRecord &record, BlockIndices block);
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

struct ReactionData
{
  struct ReactionBits
  {
    std::string in;
    std::string out;
  };

  struct Reaction
  {
    NuclideId target;
    std::list<ReactionBits> variants;
  };

  //general
  NuclideId daughter;
  BlockIndices block;
  std::string dsid;

  //header data
  std::list<Reaction> reactions;
  std::string energy;
  std::string qualifier;

  static ReactionData from_id(const IdentificationRecord &record, BlockIndices block);
  std::string to_string() const;
};


#endif
