#include "DecayScheme.h"

DecayScheme::DecayScheme(const std::string &name,
               const Nuclide &parentNuclide,
               const Nuclide &daughterNuclide,
               DecayMode DecaySchemeType)
  : decay_mode_(DecaySchemeType)
  , name_(name)
  , parent_(parentNuclide)
  , daughter_(daughterNuclide)
{
}

bool DecayScheme::valid() const
{
  return (!parent_.empty() && !daughter_.empty());
}

std::string DecayMode::to_string() const
{
  std::string ret;
  for (auto t : types_)
  {
    if (!ret.empty())
      ret += ", ";
    ret += type_to_string(t);
  }
  return ret;
}

bool DecayMode::has(DecayType t) const
{
  for (auto typ : types_)
    if (typ == t)
      return true;
  return false;
}

std::string DecayMode::type_to_string(DecayType type)
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
  case Neutron:
    return "n";
  case Proton:
    return "p";
  default:
    return "";
  }
}

std::string DecayScheme::name() const
{
  return name_;
}

DecayMode DecayScheme::mode() const
{
  return decay_mode_;
}

Nuclide DecayScheme::parentNuclide() const
{
  return parent_;
}

Nuclide DecayScheme::daughterNuclide() const
{
  return daughter_;
}

std::string DecayScheme::to_string() const
{
  std::string ret = decay_mode_.to_string() + "\n";
  ret += "Parent: " + parent_.to_string() + "\n";
  ret += "Daughter: " + daughter_.to_string() + "\n";
  return ret;
}
