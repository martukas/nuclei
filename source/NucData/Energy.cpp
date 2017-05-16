#include "Energy.h"
#include "qpx_util.h"

Energy::Energy(const UncertainDouble &v)
{
  value_ = v;
}

Energy::Energy(double energy, UncertainDouble::Sign s)
  : value_(energy, order_of(energy), s) //sigfig hack
{
  if (energy == 0)
    value_.setSymmetricUncertainty(0);
}

bool Energy::valid() const
{
  return value_.defined();
}

UncertainDouble Energy::value() const
{
  return value_;
}

Energy::operator double() const
{
  return value_;
}


std::string Energy::to_string() const
{
  if (!boost::math::isfinite(value_))
    return "";

  if (value_ >= 10000.0)
  {
    UncertainDouble mev = value_;
    mev *= 0.001;
    return mev.to_string(false) + " MeV";
  }
  return value_.to_string(false) + " keV";
}

Energy & Energy::operator=(const Energy &energy)
{
  value_ = energy.value_;
  return *this;
}

bool operator<(const Energy &left, const Energy &right)
{
  return left.value_ < right.value_;
}

bool operator<(const Energy &left, const double &right)
{
  return left.value_ < right;
}

bool operator>(const Energy &left, const Energy &right)
{
  return left.value_ > right.value_;
}

bool operator>(const Energy &left, const double &right)
{
  return left.value_ > right;
}

bool operator==(const Energy &left, const Energy &right)
{
  //  return qFuzzyCompare(left.value_, right.value_);
  return (left.value_ == right.value_);
}

Energy Energy::operator-(const Energy& other) const
{
  Energy ret = *this;
  ret.value_.setValue(value_.value() - other.value_.value());
  return ret;
}

Energy Energy::operator+(const Energy& other) const
{
  Energy ret = *this;
  ret.value_ = value_ + other.value_;
  return ret;
}

