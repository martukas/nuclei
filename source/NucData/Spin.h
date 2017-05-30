#pragma once

#include "DataQuality.h"

class Spin : public QualifiedData
{
public:
  Spin() {}
  Spin(const Spin &spin);
  Spin(uint16_t num, uint16_t denom, DataQuality q);

  bool valid();

  void set(uint16_t num, uint16_t denom = 1);
  uint16_t numerator() const;
  uint16_t denominator() const;

  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

  // some operators
  Spin& operator+=(uint16_t value);
  Spin& operator++();
  Spin operator++(int);
  Spin& operator--();
  Spin operator--(int);
  Spin operator - () const;
  Spin& operator+=(Spin sp);
  bool operator == (const Spin &s) const;
  bool operator<= (const Spin &s) const;
  bool operator< (const Spin &s) const;
  bool operator>= (const Spin &s) const;
  bool operator> (const Spin &s) const;

protected:
  uint16_t numerator_   {0};
  uint16_t denominator_ {0}; // should be 1 or 2 for a spin

  float to_float() const;
};
