#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "Spin.h"
#include "custom_logger.h"

Spin::Spin(uint16_t num, uint16_t denom)
{
  set(num, denom);
}

Spin::Spin(const Spin &spin)
  : numerator_ (spin.numerator_)
  , denominator_ (spin.denominator_)
{}


Spin& Spin::operator++()
{
  numerator_ += denominator_;
  return *this;
}

Spin Spin::operator++(int)
{
  Spin result(*this);
  Spin::operator++();
  return result;
}

Spin& Spin::operator--()
{
  numerator_ -= denominator_;
  return *this;
}

Spin Spin::operator--(int)
{
  Spin result(*this);
  Spin::operator--();
  return result;
}

Spin Spin::operator - () const
{
  Spin result(*this);
  result.set(-numerator_,denominator_);
  return result;
}

bool Spin::operator == (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return false;
}

float Spin::to_float() const
{
  return float(numerator_) / float(denominator_);
}

bool Spin::operator<= (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return to_float() <= s.to_float();
}
bool Spin::operator< (const Spin &s) const
{
  return to_float() < s.to_float();
}

bool Spin::operator>= (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return to_float() >= s.to_float();
}

bool Spin::operator> (const Spin &s) const
{
  return to_float() > s.to_float();
}

Spin Spin::from_string(const std::string& s)
{
  Spin ret;
  std::string st = strip_qualifiers(s);
  std::istringstream input; input.clear();
  if ( boost::contains(st, "/") )
  {
    // not an integer
    boost::replace_all(st, "/", " ");
    input.str(st);
    input >> ret.numerator_ >> ret.denominator_;
  }
  else
  {
    input.str(st);
    input >> ret.numerator_;
    ret.denominator_ = 1;
  }
  if ( input.fail() )
    ret.quality_ = DataQuality::kUnknown;
  else
    ret.quality_ = quality_of(s);
  return ret;
}

void Spin::set(uint16_t num, uint16_t denom)
{
  numerator_ = num; denominator_ = denom;
  if ( denominator_ < 1 || denominator_ > 2 ) {
    WARN << "Spin should be an integer or half an integer!     "
         << "[" << numerator_ << "/" << denominator_ << "]"
         << " --> "
         << "[" << numerator_ << "/1]";
    denominator_ = 1;
  }
}

Spin& Spin::operator+=(uint16_t value)
{
  numerator_ += denominator_*value;
  return *this;
}

Spin& Spin::operator+=(Spin sp)
{
  if (denominator_ == sp.denominator_) {
    numerator_ += sp.numerator_;
  } else {
    numerator_ = denominator_*sp.numerator_ + numerator_*sp.denominator_;
    denominator_ = std::max(denominator_, sp.denominator_);
  }
  return *this;
}

const std::string Spin::to_string() const
{
  if ( quality_ == DataQuality::kUnknown )
    return "";
  else if ( denominator_ == 1 )
    return std::to_string(numerator_);
  else
    return std::to_string(numerator_) + "/" + std::to_string(denominator_);
}

const std::string Spin::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}
