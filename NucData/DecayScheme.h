#ifndef DecayScheme_H
#define DecayScheme_H

#include "Nuclide.h"
#include "Transition.h"
#include "SpinParity.h"

class DecayScheme
{
public:
    enum Type {
        Undefined,
        ElectronCapture,
        BetaPlus,
        BetaMinus,
        IsomericTransition,
        Alpha
    };

    DecayScheme() {}

    DecayScheme(const std::string &name,
                    const Nuclide& parentNuclide,
                    const Nuclide& daughterNuclide,
                    Type DecayType);

    static std::string DecayTypeAsText(Type type);

    bool valid() const;
    std::string name() const;
    Type type() const;

    Nuclide parentNuclide() const;
    Nuclide daughterNuclide() const;

    std::string to_string() const;

//    Level getLevel(Energy) const;
//    Transition getTransition(Energy) const;

private:
    Type decay_type_ {Undefined};
    std::string name_;
    Nuclide parent_, daughter_;
};

#endif
