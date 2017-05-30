#pragma once

#include "Parity.h"
#include "Spin.h"
#include <vector>
#include <set>

class SpinParity
{
public:
  SpinParity() {}
  void set_parity(Parity p);
  void add_spin(Spin s);

  bool valid() const;
  std::string to_string() const;

//  friend bool operator<(const SpinParity &left, const SpinParity &right);
//  friend bool operator>(const SpinParity &left, const SpinParity &right);
//  friend bool operator==(const SpinParity &left, const SpinParity &right);

private:
  Parity parity_;
  std::vector<Spin> spins_;

  bool have_valid_spins_ {false};
  std::set<DataQuality> qs_;
};
