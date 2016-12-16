#ifndef TRANSITION_H
#define TRANSITION_H

#include "Energy.h"

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

  Energy from() const { return from_; }
  Energy to() const { return to_; }

  std::string to_string() const;

private:

  Energy energy_;
  double intensity_ {0};
  std::string multipolarity_;
  UncertainDouble delta_;
  Energy from_, to_;
};

#endif
