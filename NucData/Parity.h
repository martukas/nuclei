#ifndef PARITY_H
#define PARITY_H

#include <InfoData.h>
#include <iostream>

class Parity : public QualifiedData
{
public:
  enum class EnumParity { kMinus = -1, kPlus = 1 } ;

public:
  Parity() {}

  Parity(const Parity &other)
    : QualifiedData(other)
    , parity_(other.parity_)
  {}

  friend bool operator==(const Parity &left, const Parity &right);

  virtual void from_string(const std::string s);
  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

private:
  EnumParity  parity_ {EnumParity::kPlus};
};

#endif
