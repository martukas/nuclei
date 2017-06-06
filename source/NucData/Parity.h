#pragma once

#include "DataQuality.h"

class Parity : public QualifiedData
{
public:
  enum class EnumParity
  {
    kMinus = -1,
    kPlusMinus = 0,
    kPlus = 1
  };

public:
  Parity();
  Parity(const Parity &other);
  Parity(const EnumParity& p, const DataQuality& q);

  bool valid() const;

  friend bool operator<(const Parity &left, const Parity &right);
  friend bool operator>(const Parity &left, const Parity &right);
  friend bool operator==(const Parity &left, const Parity &right);

  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

private:
  EnumParity  parity_ {EnumParity::kPlusMinus};
};
