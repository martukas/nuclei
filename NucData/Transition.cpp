#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "Transition.h"
#include "Level.h"
#include "qpx_util.h"
#include "custom_logger.h"

Transition::Transition(Energy energy, double intensity,
                       const std::string &multipol, UncertainDouble delta,
                       Energy from, Energy to)
  : energy_(energy)
  , intensity_(intensity)
  , multipolarity_(multipol)
  , delta_(delta)
  , from_(from)
  , to_(to)
{}

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
  //  if (multipolarity_.empty())
  //    return "<i>unknown</i>";
  return multipolarity_;
}

UncertainDouble Transition::delta() const
{
  return delta_;
}

std::string Transition::intensity_string() const
{
  std::string intensstr;
  if (!boost::math::isnan(intensity_))
    intensstr = to_str_precision(intensity_, 3) + " %";
  return intensstr;
}
