#pragma once

#include "Uncert.h"
#include <map>

class HalfLife
{
public:
  HalfLife();
  HalfLife(double val, std::string units = "");
  HalfLife(Uncert time, std::string units = "");

  HalfLife preferred_units() const;

  bool isValid() const;
  double seconds() const;
  bool isStable() const;
  std::string to_string(bool with_uncert = true) const;

  bool operator>(const HalfLife &right) const;
  bool operator>=(const HalfLife &right) const;
  bool operator<(const HalfLife &right) const;

private:
  Uncert time_;
  std::string     units_;

  static const std::map<std::string, double> known_units_;
  static std::map<std::string, double> init_units();
  static std::string preferred_units(double from);
};
