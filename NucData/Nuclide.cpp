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

void Nuclide::addLevels(const std::map<Energy, Level> &levels)
{
  levels_.insert(levels.begin(), levels.end());
}

void Nuclide::addTransitions(const std::map<Energy, Transition> &tr)
{
  transitions_.insert(tr.begin(), tr.end());
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
  if (!transition.energy().valid())
  {
    WARN << "Could not add invalid transition " << transition.to_string()
        << " to " << id_.verboseName();
    return;
  }
  transitions_[transition.energy()] = transition;
}

void Nuclide::finalize()
{
  for (auto t : transitions_)
  {
    if (!levels_.count(t.second.from()) ||
        !levels_.count(t.second.to()))
    {
      WARN << "Transition cannot be linked to levels: " << t.second.to_string()
          << " for " << id_.verboseName();
      continue;
    }
    levels_[t.second.from()].addDepopulatingTransition(t.second.energy());
    levels_[t.second.to()].addPopulatingTransition(t.second.energy());
  }
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



