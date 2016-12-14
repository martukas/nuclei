#ifndef Transition_H
#define Transition_H

#include <stdint.h>
#include <memory>
#include "Energy.h"
#include "UncertainDouble.h"

class Level;

class Transition
{
public:
  Transition() {}

  Transition(Energy energy, double intensity,
             const std::string &multipol, UncertainDouble delta,
             Energy from, Energy to);

  Energy energy() const;
  std::string multipolarity() const;
  UncertainDouble delta() const;
  double intensity() const;

  std::string intensity_string() const;

  Energy depopulatedLevel() const { return from_; }
  Energy populatedLevel() const { return to_; }

private:

  Energy energy_;
  double intensity_ {0};
  std::string m_mpol;
  UncertainDouble m_delta;
  Energy from_, to_;
};

#endif
