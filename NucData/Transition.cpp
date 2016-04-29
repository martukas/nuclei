#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "Transition.h"
#include "Level.h"
#include "qpx_util.h"
#include "custom_logger.h"

Transition::Transition(Energy energy, double intensity,
                                   const std::string &multipol, UncertainDouble delta,
                                   LevelPtr start, LevelPtr dest)
  : m_e(energy),
    intens(intensity),
    m_mpol(multipol),
    m_delta(delta),
    m_start(start),
    m_dest(dest)
{
  start->m_depopulatingTransitions.push_back(TransitionPtr(this));
  dest->m_populatingTransitions.push_back(TransitionPtr(this));
}

Transition::~Transition()
{
}

Energy Transition::energy() const
{
  return m_e;
}

double Transition::intensity() const
{
  return intens;
}

std::string Transition::multipolarity() const
{
  return m_mpol;
}

const UncertainDouble & Transition::delta() const
{
  return m_delta;
}

std::string Transition::intensityAsText() const
{
  std::string intensstr;
  if (!boost::math::isnan(intens))
    intensstr = to_str_precision(intens, 3) + " %";
  return intensstr;
}

std::string Transition::multipolarityAsText() const
{
//  if (m_mpol.empty())
//    return "<i>unknown</i>";
  return m_mpol;
}

LevelPtr Transition::depopulatedLevel() const
{
  return m_start;
}

LevelPtr Transition::populatedLevel() const
{
  return m_dest;
}

