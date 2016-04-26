#ifndef XGammaTransition_H
#define XGammaTransition_H

#include <stdint.h>
#include <memory>
#include "Energy.h"
#include "UncertainDouble.h"

class XEnergyLevel;

class XGammaTransition
{
public:
    XGammaTransition(Energy energy, double intensity,
                    const std::string &multipol, UncertainDouble delta,
                    std::shared_ptr<XEnergyLevel> start, std::shared_ptr<XEnergyLevel> dest);

    virtual ~XGammaTransition();

    Energy energy() const;
    double intensity() const;
    std::string multipolarity() const;
    const UncertainDouble &delta() const;

    std::string intensityAsText() const;
    std::string multipolarityAsText() const;

    std::shared_ptr<XEnergyLevel> depopulatedLevel() const;
    std::shared_ptr<XEnergyLevel> populatedLevel() const;

    double widthFromOrigin() const;

private:
    static double gauss(const double x, const double sigma);

    Energy m_e;
    double intens;
    std::string m_mpol;
    UncertainDouble m_delta;
    std::shared_ptr<XEnergyLevel> m_start, m_dest;

};

typedef std::shared_ptr<XGammaTransition> XGammaTransitionPtr;

#endif // XGammaTransition_H
