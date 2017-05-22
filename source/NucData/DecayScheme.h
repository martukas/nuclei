#pragma once

#include "Nuclide.h"
#include "SpinParity.h"
//#include "DecayMode.h"

#include "DecayInfo.h"
#include "ReactionInfo.h"

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



  std::vector<std::string> comments;
  std::map<std::string, std::string> references_;

private:
  std::string name_;
  Nuclide parent_, daughter_;
  DecayInfo decay_info_;
  ReactionInfo reaction_info_;
};
