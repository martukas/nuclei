#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "XGammaTransition.h"
#include "XEnergyLevel.h"
#include "qpx_util.h"
#include "custom_logger.h"

XGammaTransition::XGammaTransition(Energy energy, double intensity,
                                   const std::string &multipol, UncertainDouble delta,
                                   XEnergyLevelPtr start, XEnergyLevelPtr dest)
  : m_e(energy),
    intens(intensity),
    m_mpol(multipol),
    m_delta(delta),
    m_start(start),
    m_dest(dest)
{
  start->m_depopulatingTransitions.push_back(XGammaTransitionPtr(this));
  dest->m_populatingTransitions.push_back(XGammaTransitionPtr(this));
}

XGammaTransition::~XGammaTransition()
{
}

Energy XGammaTransition::energy() const
{
  return m_e;
}

double XGammaTransition::intensity() const
{
  return intens;
}

std::string XGammaTransition::multipolarity() const
{
  return m_mpol;
}

const UncertainDouble & XGammaTransition::delta() const
{
  return m_delta;
}

std::string XGammaTransition::intensityAsText() const
{
  std::string intensstr;
  if (!boost::math::isnan(intens))
    intensstr = to_str_precision(intens, 3) + " %";
  return intensstr;
}

std::string XGammaTransition::multipolarityAsText() const
{
//  if (m_mpol.empty())
//    return "<i>unknown</i>";
  return m_mpol;
}

XEnergyLevelPtr XGammaTransition::depopulatedLevel() const
{
  return m_start;
}

XEnergyLevelPtr XGammaTransition::populatedLevel() const
{
  return m_dest;
}

