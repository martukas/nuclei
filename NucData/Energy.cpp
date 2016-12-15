#include "Energy.h"

#include <limits>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "qpx_util.h"
#include "custom_logger.h"

Energy::Energy()
{
}

Energy::Energy(double energy, UncertainDouble::Sign s)
  : value_(energy, order_of(energy), s) //sigfig hack
{
  if (energy == 0)
    value_.setSymmetricUncertainty(0);
  else
    DBG << "made energy with " << energy;
}

bool Energy::isValid() const
{
  return value_.hasFiniteValue();
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

//Energy & Energy::operator=(double energy)
//{
//  value_ = energy;
//  return *this;
//}

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


Energy Energy::operator-(Energy other)
{
  Energy ret = *this;
  ret.value_.setValue(value_.value() - other.value_.value());
  return ret;
}

Energy Energy::from_nsdf(std::string record)
{
  if (record.empty())
    return Energy();

  std::string val, uncert;
  if (record.size() >= 12)
    uncert = record.substr(10,2);
  if (record.size() >= 10)
    val = record.substr(0,10);
  else
    val = record;

  std::string offset;
  bool hasoffset = false;
  for (size_t i=0; i < val.size(); ++i) {
    if (std::isupper(val[i]) && hasoffset)
      offset += val.substr(i,1);
    else if (val[i] == '+')
      hasoffset = true;
  }
  hasoffset = hasoffset && offset.size();

  boost::replace_all(val, "+X", "");
  boost::trim(val);

  boost::trim(uncert);
  Energy ret;

  ret.value_ = UncertainDouble::from_nsdf(val, uncert);

//  tmp.remove("+Y"); // fix modified energy values (illegaly used in ensdf...)
//  double precision = get_precision(tmp.remove("+Y").toStdString());
//  double uncert = clocale.toDouble(kdestr, &convok) * precision;

//  if (hasoffset)
//  DBG << "Energy record " << record << " parsed to " << ret.value_.to_string(false, true)
//      << " offset to " << offset;
//  if (make_tentative)
//  DBG << "Energy record " << record << " parsed to " << ret.value_.to_string(false, true)
//      << " make tentative!";

  return ret;
}



