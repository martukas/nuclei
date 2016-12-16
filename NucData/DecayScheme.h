#ifndef DecayScheme_H
#define DecayScheme_H

#include "Nuclide.h"
#include "Transition.h"
#include "SpinParity.h"
#include <list>

struct DecayMode
{
  enum DecayType {
      Undefined,
      ElectronCapture,
      BetaPlus,
      BetaMinus,
      IsomericTransition,
      Alpha,
      Neutron,
      Proton
  };


  std::string to_string() const;

  std::list<DecayType> types_; //change to set

  bool has(DecayType t) const;

  static std::string type_to_string(DecayType type);
};

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

//    Level getLevel(Energy) const;
//    Transition getTransition(Energy) const;

private:
    DecayMode decay_mode_;
    std::string name_;
    Nuclide parent_, daughter_;
};

#endif
