#ifndef DECAY_SCHEME_H
#define DECAY_SCHEME_H

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

//    Level getLevel(Energy) const;
//    Transition getTransition(Energy) const;

private:
    DecayMode decay_mode_;
    std::string name_;
    Nuclide parent_, daughter_;
};

#endif
