#pragma once

#include "UncertainDouble.h"

class Energy
{
public:
  Energy() {}
  explicit Energy(const UncertainDouble &v);
  explicit Energy(double energy, UncertainDouble::Sign s);

  bool valid() const;

  std::string to_string() const;

  Energy & operator=(const Energy &energy);

  friend bool operator<(const Energy &left, const Energy &right);
  friend bool operator<(const Energy &left, const double &right);
  friend bool operator>(const Energy &left, const Energy &right);
  friend bool operator>(const Energy &left, const double &right);
  friend bool operator==(const Energy &left, const Energy &right);
  operator double() const;

  Energy operator-(Energy other);

private:
  UncertainDouble value_;
};
