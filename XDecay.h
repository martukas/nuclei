#ifndef XDecay_H
#define XDecay_H

#include <stdint.h>
#include "XNuclide.h"
#include "XGammaTransition.h"
#include "SpinParity.h"

class XEnergyLevel;

class XDecay
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

    XDecay();

    XDecay(const std::string &name,
                    XNuclidePtr parentNuclide,
                    XNuclidePtr daughterNuclide,
                    Type DecayType);

    static std::string DecayTypeAsText(Type type);

    bool valid() const;
    std::string name() const;
    Type type() const;

    XNuclidePtr parentNuclide() const;
    XNuclidePtr daughterNuclide() const;

    XEnergyLevelPtr getLevel(Energy) const;
    XGammaTransitionPtr getTransition(Energy) const;

private:
    std::string m_name;
    XNuclidePtr pNuc, dNuc;
    Type t;
};

typedef std::shared_ptr<XDecay> XDecayPtr;

#endif // XDecay_H
