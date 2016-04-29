#ifndef DecayScheme_H
#define DecayScheme_H

#include <stdint.h>
#include "Nuclide.h"
#include "Transition.h"
#include "SpinParity.h"

class Level;

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

    DecayScheme();

    DecayScheme(const std::string &name,
                    NuclidePtr parentNuclide,
                    NuclidePtr daughterNuclide,
                    Type DecayType);

    static std::string DecayTypeAsText(Type type);

    bool valid() const;
    std::string name() const;
    Type type() const;

    NuclidePtr parentNuclide() const;
    NuclidePtr daughterNuclide() const;

    LevelPtr getLevel(Energy) const;
    TransitionPtr getTransition(Energy) const;

private:
    Type t;
    std::string m_name;
    NuclidePtr pNuc, dNuc;
};

typedef std::shared_ptr<DecayScheme> DecaySchemePtr;

#endif // DecayScheme_H
