#pragma once

#include <NucData/Nuclide.h>
#include <NucData/DecayInfo.h>
#include <NucData/ReactionInfo.h>

#include <nlohmann/json.hpp>
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
  void add_text(const std::string& heading, const json &j);

private:
  std::string name_;
  Nuclide parent_, daughter_;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;

  json text_;
  std::set<std::string> references_;
};
