#pragma once

#include "Parity.h"
#include "Spin.h"
#include <vector>

class SpinParity
{
public:
  SpinParity() {}
  void set_parity(Parity p);
  void add_spin(Spin s);

  bool valid() const;
  std::string to_string() const;

private:
  Parity parity_;
  std::vector<Spin> spins_;
};
