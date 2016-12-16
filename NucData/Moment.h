#ifndef MOMENT_H
#define MOMENT_H

#include "UncertainDouble.h"
#include <vector>

class Moment
{
public:
  Moment() {}
  Moment(const Moment &p) { moment_ = p.moment_; }
  Moment(const UncertainDouble &v) { moment_ = v; }

  bool valid() const;

  void add_reference(std::string ref) { references_.push_back(ref); }
  void set_references(std::vector<std::string> refs) { references_ = refs; }

  const std::string to_string() const;
  const std::string to_markup() const;

private:
  UncertainDouble moment_;
  std::vector<std::string> references_; // TODO? not converted to string
};

#endif
