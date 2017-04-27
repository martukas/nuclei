#pragma once

#include "DecayScheme.h"
#include <list>

struct BlockIndices
{
  BlockIndices() {}
  BlockIndices(size_t first, size_t last)
    : first(first), last(last) {}
  void clear() { first = last = 0; }
  size_t first {0};
  size_t last {0};
};


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
  static CommentsRecord from_id(const IdentificationRecord &record,
                                BlockIndices block);

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
  static BasicDecayData from_id(const IdentificationRecord &record,
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

struct ReactionData
{
  struct Reactants
  {
    std::string in;
    std::string out;

    std::string to_string() const;
    void parse_reactants(std::string s);
  };

  struct Reaction
  {
    NuclideId target;
    std::list<Reactants> variants;

    std::string to_string() const;
    void parse_reaction(std::string s);
  };

  //general
  NuclideId daughter;
  BlockIndices block;
  std::string dsid;

  //header data
  std::list<Reaction> reactions;
  std::string energy;
  std::string qualifier;

  static bool match(std::string record);
  bool find_remove(std::string &extras, std::string wanted, std::string trim_what);
  static ReactionData from_id(const IdentificationRecord &record,
                              BlockIndices block);
  std::string to_string() const;
};
