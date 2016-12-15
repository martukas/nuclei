#ifndef MOMENT_H
#define MOMENT_H

#include <iostream>
#include "UncertainDouble.h"
#include <vector>
#include <string>

class Moment
{
public:
  Moment() {}
  Moment(const Moment &p) { moment_ = p.moment_; }
  static Moment from_ensdf(std::string s);

  bool valid() const;

  const std::string to_string() const;
  const std::string to_markup() const;

private:
  UncertainDouble moment_;
  std::vector<std::string> references_; // TODO? not converted to string
};

#endif
