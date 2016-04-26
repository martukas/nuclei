#ifndef XDecay_H
#define XDecay_H

#include <stdint.h>
#include "XNuclide.h"
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

    std::string name() const;
    Type type() const;

    XNuclidePtr parentNuclide() const;
    XNuclidePtr daughterNuclide() const;

    struct CascadeIdentifier {
        CascadeIdentifier();
        Energy start;
        Energy pop;
        Energy intermediate;
        bool highlightIntermediate;
        Energy depop;
    };

private:
    std::string m_name;
    XNuclidePtr pNuc, dNuc;
    Type t;
};

#endif // XDecay_H
