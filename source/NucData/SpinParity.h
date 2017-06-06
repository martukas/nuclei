#pragma once

#include "Parity.h"
#include "Spin.h"
#include "Uncert.h"
#include <vector>
#include <set>

class SpinParity
{
public:
  SpinParity() {}
  void set_parity(Parity p);
  void set_spin(Spin s);
  void set_common_quality(DataQuality);
  void set_eq_type(Uncert::UncertaintyType t);
  Spin spin() const;
  Parity parity() const;

  bool valid() const;
  std::string to_string(bool with_qualifiers) const;

//  friend bool operator<(const SpinParity &left, const SpinParity &right);
//  friend bool operator>(const SpinParity &left, const SpinParity &right);
//  friend bool operator==(const SpinParity &left, const SpinParity &right);

private:
  Parity parity_;
  Spin spin_;
  Uncert::UncertaintyType eq_type_ {Uncert::UndefinedType};
};


class SpinSet
{
public:
  SpinSet() {}
  void add(SpinParity sp);
  void set_common_quality(DataQuality);
  void set_common_parity(Parity);
  void merge(const SpinSet&);

  bool valid() const;
  std::string to_string() const;
  std::string to_pretty_string() const;

  std::string debug() const;

  std::string logic() const;
  void set_logic(std::string);

private:
  std::vector<SpinParity> sps_;

  std::set<Parity> parities_;
  std::set<Spin> spins_;
  std::set<DataQuality> qual_spins_;
  std::set<DataQuality> qual_parities_;

  std::string logical_ {"%"};
};
