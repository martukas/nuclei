#include "XDecay.h"
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <algorithm>
#include "custom_logger.h"

XDecay::XDecay()
  : m_name("")
  , t(Undefined)
  , pNuc(0)
  , dNuc(0)
{

}

XDecay::XDecay(const std::string &name,
               XNuclidePtr parentNuclide,
               XNuclidePtr daughterNuclide,
               Type XDecayType)
  : m_name(name)
  , pNuc(parentNuclide)
  , dNuc(daughterNuclide)
  , t(XDecayType)
{
}

std::string XDecay::DecayTypeAsText(Type type)
{
  switch (type) {
  case ElectronCapture:
    return "Electron Capture";
  case BetaPlus:
    return "β+";
  case BetaMinus:
    return "β-";
  case IsomericTransition:
    return "Isomeric Transition";
  case Alpha:
    return "α";
  default:
    return "";
  }
}

std::string XDecay::name() const
{
  return m_name;
}

XDecay::Type XDecay::type() const
{
  return t;
}

XNuclidePtr XDecay::parentNuclide() const
{
  return pNuc;
}

XNuclidePtr XDecay::daughterNuclide() const
{
  return dNuc;
}
