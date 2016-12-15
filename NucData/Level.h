#ifndef LEVEL_H
#define LEVEL_H

#include <limits>
#include <set>
#include "HalfLife.h"
#include "Energy.h"
#include "SpinParity.h"
#include "Moment.h"

class Level
{
public:

  Level() {}

  Level(Energy energy, SpinParity spin,
        HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
        uint16_t isomerNum = 0);

  static Level from_ensdf(std::string record);

  Energy energy() const;
  SpinParity spin() const;
  HalfLife halfLife() const;
  uint16_t isomerNum() const;
  UncertainDouble normalizedFeedIntensity() const;
  Moment mu() const;
  Moment q() const;

  void set_mu(const Moment &);
  void set_q(const Moment &);
  void set_halflife(const HalfLife&);
  void set_spin(const SpinParity&);

  std::set<Energy> depopulatingTransitions() const { return depopulating_transitions_; }
  std::set<Energy> populatingTransitions() const { return populating_transitions_; }

  void addPopulatingTransition(const Energy&);
  void addDepopulatingTransition(const Energy&);

  std::string to_string() const;

  void setFeedIntensity(UncertainDouble intensity);
  void setFeedingLevel(bool isfeeding);

  bool isFeedingLevel() const;


private:
  Energy energy_;
  SpinParity spin_;
  HalfLife halflife_;
  Moment q_, mu_; // quadrupole and magnetic moments
  uint16_t isomeric_ {0}; // >0 for isomeric levels (counted from low energy to high), 0 otherwise

  UncertainDouble feeding_intensity_; // says how often this level is directly fed per 100 parent decays
  bool feeding_level_ {false}; // true if this is belonging to a parent nuclide and is a starting point for decays

  std::set<Energy> populating_transitions_;
  std::set<Energy> depopulating_transitions_;
};

#endif
