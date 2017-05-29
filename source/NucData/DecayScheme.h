#pragma once

#include "Nuclide.h"
#include "DecayInfo.h"
#include "ReactionInfo.h"

#include "json.h"
using namespace nlohmann;

class DecayScheme
{
public:
  DecayScheme() {}
  DecayScheme(const std::string &name,
              const Nuclide& parentNuclide,
              const Nuclide& daughterNuclide,
              DecayInfo decayinfo,
              ReactionInfo reactinfo);

  bool valid() const;
  std::string name() const;

  DecayInfo decay_info() const;
  ReactionInfo reaction_info() const;

  Nuclide parentNuclide() const;
  Nuclide daughterNuclide() const;

  std::string to_string() const;


  std::set<std::string> references() const;
  void insert_reference(const std::string &s);

  json text() const;
  void add_text(const json &j);

private:
  std::string name_;
  Nuclide parent_, daughter_;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

  json text_;
  std::set<std::string> references_;
};
