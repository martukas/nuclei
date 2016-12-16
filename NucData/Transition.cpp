#include "Transition.h"
#include <boost/math/special_functions/fpclassify.hpp>
#include "qpx_util.h"

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
  return multipolarity_;
}

UncertainDouble Transition::delta() const
{
  return delta_;
}

std::string Transition::intensity_string() const
{
  std::string intensstr;
  if (boost::math::isfinite(intensity_))
    intensstr = to_str_precision(intensity_, 3) + " %";
  return intensstr;
}

std::string Transition::to_string() const
{
  std::stringstream ss;
  ss << std::setw(16) << energy_.to_string() << "   "
     << std::setw(15) << from_.to_string()
     << " --> "
     << std::setw(15) << to_.to_string()
     << std::setw(7)  << intensity_string()
     << std::setw(12) << multipolarity_;
  if (delta_.hasFiniteValue())
    ss << "  delta="  << delta_.to_string(false);
  return ss.str();
}
