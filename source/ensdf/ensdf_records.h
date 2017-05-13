#pragma once

#include "DecayScheme.h"
#include <list>
#include <cmath>

struct BlockIndices
{
  BlockIndices() {}
  BlockIndices(size_t first, size_t last)
    : first(first), last(last) {}
  void clear() { first = last = 0; }
  size_t first {0};
  size_t last {0};
};

template <typename T>
Energy findNearest(const std::map<Energy, T> &map,
                   const Energy &val)
{
  if (map.empty())
    return Energy();
  typename std::map<Energy, T>::const_iterator low, prev;
  low = map.lower_bound(val);
  if (low == map.end())
    low--;
  else if (low != map.begin()) {
    prev = low;
    --prev;
    if (std::abs(val - prev->first) < std::abs(low->first - val))
      low = prev;
  }
  return low->first;
}

bool match_record_type(const std::string& line,
                       const std::string& pattern);

std::vector<std::string> extractContinuationRecords(const BlockIndices &adoptedblock,
                                                    const std::list<std::string> &requestedRecords,
                                                    const std::vector<std::string>& data,
                                                    std::string typeOfContinuedRecord = "L");

std::map<Energy, std::string> get_gamma_lines(const std::vector<std::string>& data,
                                              BlockIndices bounds, NuclideId nucid);

bool is_gamma_line(const std::string& line,
                   std::string nucid,
                   std::string nucid2 = "");

bool is_level_line(const std::string& line,
                   std::string nucid,
                   std::string nucid2 = "");

bool is_intensity_line(const std::string& line,
                       std::string nucid,
                       std::string nucid2 = "");

bool is_norm_line(const std::string& line,
                  std::string nucid,
                  std::string nucid2 = "");

bool is_p_norm_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_a_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_b_line(const std::string& line,
                    std::string nucid,
                    std::string nucid2 = "");

bool is_feed_line(const std::string& line,
                  std::string nucid,
                  std::string nucid2 = "");

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

struct HistoryRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}[\\s02-9A-Za-z]\\sH.*$");
  }

  static HistoryRecord parse(size_t& idx,
                             const std::vector<std::string>& data);

  NuclideId nuc_id;
  std::map<std::string, std::string> kvps;
};

struct QValueRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}\\s{2}Q\\s.*$");
  }
};

struct XRefRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}\\s{2}X.*$");
  }
};

struct NormalizationRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z]{5}\\s{2}N.*$");
  }
};

struct ProdNormalizationRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z]{6}PN.*$");
  }
};

struct LevelRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z$@]{6}\\sL\\s.*$");
  }
};

struct AlphaRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{6}\\sA\\s.*$");
  }
};

struct BetaRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{6}\\sB\\s.*$");
  }
};

struct GammaRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}.\\sG\\s.*$");
  }
};

struct ParticleRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{6}\\s[\\sD][NPA].*$");
  }
};

struct ECRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{6}\\sE\\s.*$");
  }
};

struct ReferenceRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9]{3}\\s{4}R\\s.*$");
  }
};

struct CommentsRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Za-z]{5}.[cdtCDT].*$");
  }

  static CommentsRecord from_id(const IdRecord &record,
                                BlockIndices block);

  NuclideId nuclide;
  BlockIndices block;
};

struct ParentRecord
{
  static bool is(const std::string& line)
  {
    return match_record_type(line,
                             "^[\\s0-9A-Z]{5}\\s{2}P.*$");
  }

  NuclideId nuclide;
  Energy energy;
  HalfLife hl;
  SpinParity spin;

  static ParentRecord from_ensdf(const std::string &precstr);
  std::string to_string() const;
};

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
  static ReactionData from_id(const IdRecord &record,
                              BlockIndices block);
  std::string to_string() const;
};
