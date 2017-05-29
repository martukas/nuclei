#pragma once

#include "Uncert.h"

class Energy
{
public:
  Energy() {}
  explicit Energy(const Uncert &v);
  explicit Energy(double energy, Uncert::Sign s);

  bool valid() const;
  Uncert value() const;

  std::string to_string() const;

  Energy & operator=(const Energy &energy);

  friend bool operator<(const Energy &left, const Energy &right);
  friend bool operator<(const Energy &left, const double &right);
  friend bool operator>(const Energy &left, const Energy &right);
  friend bool operator>(const Energy &left, const double &right);
  friend bool operator==(const Energy &left, const Energy &right);
  operator double() const;

  Energy operator-(const Energy& other) const;
  Energy operator+(const Energy& other) const;

private:
  Uncert value_;
};
