#pragma once

#include <limits>
#include <set>
#include "HalfLife.h"
#include "Energy.h"
#include "SpinParity.h"
#include "Moment.h"

#include "json.h"
using namespace nlohmann;

class Level
{
public:
  Level() {}
  Level(Energy energy, SpinParity spin,
        HalfLife halfLife = HalfLife(std::numeric_limits<double>::infinity()),
        uint16_t isomerNum = 0);

  Energy energy() const;
  SpinParity spin() const;
  HalfLife halfLife() const;
  uint16_t isomerNum() const;
  Uncert normalizedFeedIntensity() const;
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

  void setFeedIntensity(Uncert intensity);
  void setFeedingLevel(bool isfeeding);

  bool isFeedingLevel() const;

//  std::map<std::string, std::string> kvps;

  json text() const;
  void add_text(const std::string& heading, const json &j);

private:
  Energy energy_;
  SpinParity spin_;
  HalfLife halflife_;
  Moment q_, mu_; // quadrupole and magnetic moments
  uint16_t isomeric_ {0}; // >0 for isomeric levels (counted from low energy to high), 0 otherwise

  Uncert feeding_intensity_; // says how often this level is directly fed per 100 parent decays
  bool feeding_level_ {false}; // true if this is belonging to a parent nuclide and is a starting point for decays

  std::set<Energy> populating_transitions_;
  std::set<Energy> depopulating_transitions_;

  json text_;
};
