#include "Nuclide.h"
#include "qpx_util.h"
#include "custom_logger.h"

Nuclide::Nuclide()
{
}

Nuclide::Nuclide(NuclideId id)
  : id_(id)
{
}

NuclideId Nuclide::id() const
{
  return id_;
}

bool Nuclide::empty() const
{
  return (!id_.valid() || levels_.empty() /*|| halflives_.empty()*/);
}

std::map<Energy, Level> Nuclide::levels() const
{
  return levels_;
}

std::map<Energy, Transition> Nuclide::transitions() const
{
  return transitions_;
}

void Nuclide::setHalflives(const std::vector<HalfLife>& hl)
{
  halflives_ = hl;
}

void Nuclide::addHalfLife(const HalfLife& hl)
{
  halflives_.push_back(hl);
}

std::vector<HalfLife> Nuclide::halfLifes() const
{
  return halflives_;
}

std::string Nuclide::halfLifeAsText() const
{
  std::vector<std::string> results;
  for (auto &h : halflives_)
    if (h.isValid() && !h.isStable())
      results.push_back(h.to_string());
  return join(results, ", ");
}

void Nuclide::addLevel(const Level& level)
{
  if (level.energy().valid())
    levels_[level.energy()] = level;
}

void Nuclide::addTransition(const Transition& transition)
{
  if (transition.energy().valid())
    transitions_[transition.energy()] = transition;
//  else
//    WARN << "Could not add invalid transition " << transition.to_string()
//        << " to " << id_.verboseName();
}

void Nuclide::addNewTransition(const Energy& energy,
                               const Energy& to,
                               UncertainDouble intensity)
{
  if (levels_.count(to))
  {
    Energy from = to + energy;
    Transition transition(energy, intensity,
                          std::string(), UncertainDouble(),
                          from, to);
    transitions_[transition.energy()] = transition;
    if (levels_.count(from))
      levels_[from] = Level(from, SpinParity());
    registerTransition(transition);
  }
}

void Nuclide::removeTransition(const Transition& t)
{
  if (transitions_.count(t.energy()))
    transitions_.erase(t.energy());
}

void Nuclide::finalize()
{
  for (auto t : transitions_)
    if (levels_.count(t.second.from()) && levels_.count(t.second.to()))
      registerTransition(t.second);
//    else
//      WARN << "Transition cannot be linked to levels: " << t.second.to_string()
//          << " for " << id_.verboseName();
}

void Nuclide::cullLevels()
{
  for (auto l : levels_)
    if (!hasTransitions(l.first))
      levels_.erase(l.first);
}

bool Nuclide::hasTransitions(const Energy& level) const
{
  for (auto t : transitions_)
    if ((level == t.second.from()) || (level == t.second.to()))
      return true;
  return false;
}

void Nuclide::registerTransition(const Transition& t)
{
  levels_[t.from()].addDepopulatingTransition(t.energy());
  levels_[t.to()].addPopulatingTransition(t.energy());
}

std::string Nuclide::to_string() const
{
  std::string ret = id_.symbolicName() + " ";
  for (auto h : halflives_)
    ret += h.to_string();
  ret += "\n";
  if (levels_.size())
  {
    ret += "Levels (" + boost::lexical_cast<std::string>(levels_.size()) +")\n";
    for (auto l : levels_)
      ret += l.second.to_string() +  "\n";
  }
  if (transitions_.size())
  {
    ret += "Transitions (" + boost::lexical_cast<std::string>(transitions_.size()) + ")\n";
    for (auto t : transitions_)
      ret += "  " + t.second.to_string() +  "\n";
  }
  return ret;
}



