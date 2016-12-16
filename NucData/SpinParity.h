#ifndef SPINPARITY_H
#define SPINPARITY_H

#include "Parity.h"
#include "Spin.h"
#include <vector>

class SpinParity
{
public:
  SpinParity() {}
  void set_parity(Parity p)
  {
    parity_ = p;
  }

  void add_spin(Spin s)
  {
    spins_.push_back(s);
  }

  bool valid() const;

  std::string to_string() const;

private:
  Parity parity_;
  std::vector<Spin> spins_;
};

#endif
