#include "DecayScheme.h"
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include <algorithm>
#include "custom_logger.h"

DecayScheme::DecayScheme()
  : t(Undefined)
  , m_name("")
  , pNuc(0)
  , dNuc(0)
{

}

DecayScheme::DecayScheme(const std::string &name,
               NuclidePtr parentNuclide,
               NuclidePtr daughterNuclide,
               Type DecaySchemeType)
  : t(DecaySchemeType)
  , m_name(name)
  , pNuc(parentNuclide)
  , dNuc(daughterNuclide)
{
}

bool DecayScheme::valid() const
{
  return (pNuc && dNuc && !pNuc->empty() && !dNuc->empty());
}

std::string DecayScheme::DecayTypeAsText(Type type)
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

std::string DecayScheme::name() const
{
  return m_name;
}

DecayScheme::Type DecayScheme::type() const
{
  return t;
}

NuclidePtr DecayScheme::parentNuclide() const
{
  return pNuc;
}

NuclidePtr DecayScheme::daughterNuclide() const
{
  return dNuc;
}
