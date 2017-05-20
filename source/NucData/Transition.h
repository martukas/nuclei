#pragma once

#include "Energy.h"

class Transition
{
public:
  Transition() {}

  Transition(Energy energy, UncertainDouble intensity,
             const std::string &multipol, UncertainDouble delta,
             Energy from, Energy to);

  Energy energy() const;
  Energy from() const;
  Energy to() const;

  UncertainDouble intensity() const;
  std::string intensity_string() const;
  std::string multipolarity() const;
  UncertainDouble delta() const;

  std::string to_string() const;

private:
  Energy energy_;
  UncertainDouble intensity_;
  std::string multipolarity_;
  UncertainDouble delta_;
  Energy from_, to_;
};
