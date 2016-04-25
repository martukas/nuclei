#ifndef SPINPARITY_H
#define SPINPARITY_H

#include "Parity.h"
#include "Spin.h"
#include <vector>

class SpinParity
{
public:

  SpinParity();

  static SpinParity from_ensdf(std::string data);

  bool valid() const;

  int doubled_spin() const;
  std::string to_string() const;

private:
  Parity parity_;
  std::vector<Spin> spins_;
};

#endif // SPINPARITY_H
