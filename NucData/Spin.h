#ifndef SPIN_H
#define SPIN_H

#include "InfoData.h"
#include <iostream>

class Spin : public QualifiedData
{
public:
  Spin() {}
  Spin(uint16_t num, uint16_t denom);
  Spin(const Spin &spin);
  static Spin from_string(const std::string& s); //from_ensdf?

  void set(uint16_t num, uint16_t denom = 1);
  uint16_t numerator()   const  { return numerator_; }
  uint16_t denominator() const  { return denominator_; }

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
  uint16_t numerator_ {0};
  uint16_t denominator_ {1}; // should be 1 or 2 for a spin

  float to_float() const;
};

#endif
