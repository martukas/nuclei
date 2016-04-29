#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "Transition.h"
#include "Level.h"
#include "qpx_util.h"
#include "custom_logger.h"

Transition::Transition(Energy energy, double intensity,
                                   const std::string &multipol, UncertainDouble delta,
                                   LevelPtr start, LevelPtr dest)
  : energy_(energy),
    intensity_(intensity),
    m_mpol(multipol),
    m_delta(delta),
    from_(start),
    to_(dest)
{
  start->m_depopulatingTransitions.push_back(TransitionPtr(this));
  dest->m_populatingTransitions.push_back(TransitionPtr(this));
}

Transition::~Transition()
{
}

Energy Transition::energy() const
{
  return energy_;
}

double Transition::intensity() const
{
  return intensity_;
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
  if (!boost::math::isnan(intensity_))
    intensstr = to_str_precision(intensity_, 3) + " %";
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
  return from_;
}

LevelPtr Transition::populatedLevel() const
{
  return to_;
}

