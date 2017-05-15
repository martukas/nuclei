#pragma once

#include "id_record.h"
#include "parent_record.h"
#include "DecayScheme.h"

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
