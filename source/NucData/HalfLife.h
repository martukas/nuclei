#pragma once

#include "Uncert.h"
#include <map>

class HalfLife
{
public:
  HalfLife();
  HalfLife(double time, bool approx, std::string units = "");
  HalfLife(Uncert time, bool approx, std::string units = "");

  HalfLife preferred_units() const;

  bool valid() const;

  Uncert time() const;
  std::string units() const;
  bool tentative() const;

  double seconds() const;
  double ev() const;
  bool stable() const;
  std::string to_string(bool with_uncert = true) const;

  bool operator>(const HalfLife &right) const;
  bool operator>=(const HalfLife &right) const;
  bool operator<(const HalfLife &right) const;

private:
  Uncert       time_;
  std::string  units_;
  bool         tentative_ {false};

  static const std::map<std::string, double> time_units_;
  static const std::map<std::string, double> e_units_;
  static std::map<std::string, double> init_time_units();
  static std::map<std::string, double> init_e_units();
  static std::string preferred_time_units(double from);
  static std::string preferred_e_units(double from);
};
