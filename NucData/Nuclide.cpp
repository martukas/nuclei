#include "Nuclide.h"
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"
#include "qpx_util.h"

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
  if (level.energy().isValid())
    levels_[level.energy()] = level;
}

void Nuclide::addTransition(const Transition& transition)
{
  if (transition.energy().isValid()
//      && levels_.count(transition.populatedLevel())
//      && levels_.count(transition.depopulatedLevel())
      )
  {
    transitions_[transition.energy()] = transition;
    levels_[transition.depopulatedLevel()].addPopulatingTransition(transition.energy());
    levels_[transition.populatedLevel()].addDepopulatingTransition(transition.energy());
  }
}


