#pragma once

#include <NucData/Uncert.h>
#include <vector>

class Moment
{
public:
  Moment() {}
  Moment(const Moment &p);
  Moment(const Uncert &v);

  bool valid() const;

  void add_reference(std::string ref);
  void set_references(std::vector<std::string> refs);

  const std::string to_string() const;
  const std::string to_markup() const;

private:
  Uncert moment_;
  std::vector<std::string> references_; // TODO? not converted to string
};
