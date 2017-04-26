#pragma once

#include "Nuclide.h"
#include "SpinParity.h"
#include "DecayMode.h"

class DecayScheme
{
public:
  DecayScheme() {}
  DecayScheme(const std::string &name,
              const Nuclide& parentNuclide,
              const Nuclide& daughterNuclide,
              DecayMode DecayType);

  bool valid() const;
  std::string name() const;
  DecayMode mode() const;

  Nuclide parentNuclide() const;
  Nuclide daughterNuclide() const;

  std::string to_string() const;

private:
  DecayMode decay_mode_;
  std::string name_;
  Nuclide parent_, daughter_;
};
