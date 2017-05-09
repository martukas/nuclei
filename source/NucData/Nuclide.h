#pragma once

#include "nid.h"
#include "Level.h"
#include "Transition.h"

class Nuclide
{
public:
  Nuclide();
  Nuclide(NuclideId id);

  NuclideId id() const;

  bool empty() const;

  void addLevel(const Level& level);
  void addTransition(const Transition& transition);
  void finalize();

  void addNewTransition(const Energy& energy,
                        const Energy& to,
                        double intensity);

  void removeTransition(const Transition& t);

  void cullLevels();
  bool hasTransitions(const Energy& level) const;

  std::map<Energy, Level> levels() const;
  std::map<Energy, Transition> transitions() const;

  void setHalflives(const std::vector<HalfLife>& hl);
  void addHalfLife(const HalfLife& hl);

  std::vector<HalfLife> halfLifes() const;
  std::string halfLifeAsText() const;

  std::string to_string() const;

private:
  NuclideId   id_;
  std::vector <HalfLife> halflives_;
  std::map<Energy, Level> levels_;
  std::map<Energy, Transition> transitions_;

  void registerTransition(const Transition& t);
};
