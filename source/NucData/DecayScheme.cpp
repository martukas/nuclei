#include <NucData/DecayScheme.h>

DecayScheme::DecayScheme(const std::string &name,
                         const Nuclide &parentNuclide,
                         const Nuclide &daughterNuclide,
                         DecayInfo decayinfo, ReactionInfo reactinfo)
  : name_(name)
  , parent_(parentNuclide)
  , daughter_(daughterNuclide)
  , decay_info_(decayinfo)
  , reaction_info_(reactinfo)
{
}

bool DecayScheme::valid() const
{
  return (!daughter_.empty() || !text_.empty());
}

void DecayScheme::insert_reference(const std::string &s)
{
  references_.insert(s);
}

void DecayScheme::add_text(const std::string &heading, const json &j)
{
  json block;
  block["heading"] = heading;
  block["pars"] = j;
  text_.push_back(block);
}

json DecayScheme::text() const
{
  return text_;
}

std::set<std::string> DecayScheme::references() const
{
  return references_;
}

std::string DecayScheme::name() const
{
  return name_;
}

DecayInfo DecayScheme::decay_info() const
{
  return decay_info_;
}

ReactionInfo DecayScheme::reaction_info() const
{
  return reaction_info_;
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
  std::string ret;
  ret += "Parent: " + parent_.to_string() + "\n";
  ret += "Daughter: " + daughter_.to_string() + "\n";
  if (decay_info_.valid())
    ret += "Decay: " + decay_info_.name() + "\n";
  if (decay_info_.valid())
    ret += "React: " + reaction_info_.name() + "\n";
  return ret;
}
