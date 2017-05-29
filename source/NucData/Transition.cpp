#include "Transition.h"
#include <boost/math/special_functions/fpclassify.hpp>
#include "qpx_util.h"

Transition::Transition(Energy energy, Uncert intensity)
  : energy_(energy)
  , intensity_(intensity)
{}

void Transition::set_to(const Energy& e)
{
  to_ = e;
}

void Transition::set_from(const Energy& e)
{
  from_ = e;
}

void Transition::set_multipol(const std::string& s)
{
  multipolarity_ = s;
}

void Transition::set_delta(const Uncert& u)
{
  delta_ = u;
}

Energy Transition::energy() const
{
  return energy_;
}

Energy Transition::from() const
{
  return from_;
}

Energy Transition::to() const
{
  return to_;
}

Uncert Transition::intensity() const
{
  return intensity_;
}

std::string Transition::multipolarity() const
{
  return multipolarity_;
}

Uncert Transition::delta() const
{
  return delta_;
}

std::string Transition::intensity_string() const
{
  if (intensity_.defined())
    return intensity_.to_string(false) + "%";
  else
    return intensity_.to_string(false);
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

void Transition::add_comments(const std::string &s, const json &j)
{
  comments_[s] = j;
}

json Transition::comments() const
{
  return comments_;
}

