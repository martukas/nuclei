#pragma once

#include "nid.h"
#include <list>

struct Reactants
{
  Reactants() {}
  Reactants(std::string s);

  bool valid() const;
  std::string to_string() const;

  std::string in;
  std::string out;
};

struct Reaction
{
  Reaction() {}
  Reaction(std::string s, NuclideId daughter);

  bool valid() const;
  std::string to_string() const;

  NuclideId target;
  std::list<Reactants> variants;
};

struct ReactionInfo
{
  ReactionInfo() {}
  ReactionInfo(std::string ext_dsid, NuclideId daughter);

  static bool match(std::string record);

  bool valid() const;
  std::string to_string() const;
  std::string name() const;

  //header data
  std::list<Reaction> reactions;
  std::string energy;
  std::string qualifier;

private:
  bool find_remove(std::string &extras,
                   std::string wanted,
                   std::string trim_what);
};

