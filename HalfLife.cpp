#include "HalfLife.h"

#include <limits>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>
#include "qpx_util.h"
#include "custom_logger.h"

HalfLife::HalfLife()
  : HalfLife(std::numeric_limits<double>::quiet_NaN())
{
}

HalfLife::HalfLife(double seconds)
  : time_(seconds, order_of(seconds), UncertainDouble::SignMagnitudeDefined) //sigfig hack
//  , units_("s")
{
}

HalfLife HalfLife::from_ensdf(std::string record)
{
  std::string value_str;
  std::string units_str;
  std::string uncert_str;

  std::vector<std::string> timeparts;
  std::string rec_copy = trim_all(record);
  boost::split(timeparts, rec_copy, boost::is_any_of(" \r\t\n\0"));
  if (timeparts.size() >= 1)
    value_str = timeparts[0];
  if (timeparts.size() >= 2)
    units_str = timeparts[1];
  if (timeparts.size() >= 3)
    uncert_str = timeparts[2];

  UncertainDouble awesome = UncertainDouble::from_nsdf(value_str, uncert_str);
  double conversion = 1.0;
  if (known_units_.count(units_str))
    conversion = known_units_.at(units_str);
  UncertainDouble final = awesome;
  final *= conversion;
  std::string preferred = preferred_units(final.value());

//  INFO << "       " << awesome.to_string(false, true) << " x " << conversion
//      << " = " << final.to_string(false, true) <<  " preferred as " << preferred;

  final *= (1/known_units_.at(preferred));

  if (boost::contains(boost::to_upper_copy(record), "STABLE"))
    final.setValue(std::numeric_limits<double>::infinity(), UncertainDouble::SignMagnitudeDefined);
  else if (boost::contains(record, "EV"))
    final.setValue(std::numeric_limits<double>::quiet_NaN(), UncertainDouble::SignMagnitudeDefined);

  HalfLife ret;
  ret.time_ = final;
  ret.units_ = preferred;

//  INFO << "Haflife  " << record << "     --> " << ret.to_string();
  return ret;
}

bool HalfLife::isValid() const
{
//  INFO << "HL " << to_string() << " valid=" << !boost::math::isnan(time_.value());
  return !boost::math::isnan(time_.value());
}

double HalfLife::seconds() const
{
  UncertainDouble tmp = time_;
  double factor = 1.0;
  if (known_units_.count(units_))
    factor = known_units_.at(units_);
  tmp *= factor;
//  INFO << "HL " << to_string() << " s=" << tmp.value();
  return tmp.value();
}

bool HalfLife::isStable() const
{
//  INFO << "HL " << to_string() << " stable=" << boost::math::isinf(time_.value());
  return boost::math::isinf(time_.value());
}

std::string HalfLife::to_string() const
{
  if (!isValid())
    return "";
  if (isStable())
    return "stable";
  return time_.to_string(false) + units_;
}

std::string HalfLife::preferred_units(double secs)
{
  if (secs > 86400. * 365. * 2.)
    return "y";
  else if (secs > 86400.* 2.)
    return "d";
  else if (secs > 3600.* 2.)
    return "h";
  else if (secs > 60.* 2.)
    return "m";
  else if (secs < 1.E-15)
    return "as";
  else if (secs < 1.E-12)
    return "fs";
  else if (secs < 1.E-9)
    return "ps";
  else if (secs < 1.E-6)
    return "ns";
  else if (secs < 1.E-3)
    return "µs";
  else if (secs < 1.)
    return "ms";
  else
    return "s";
}

bool HalfLife::operator >(const HalfLife &right) const
{
//  INFO << "HLC " << to_string() << ">" << right.to_string() << " " << (seconds() > right.seconds());
  return seconds() > right.seconds();
}

bool HalfLife::operator >=(const HalfLife &right) const
{
//  INFO << "HLC " << to_string() << ">=" << right.to_string() << " " << (seconds() >= right.seconds());
  return seconds() >= right.seconds();
}

bool HalfLife::operator <(const HalfLife &right) const
{
//  INFO << "HLC " << to_string() << "<" << right.to_string() << " " << (seconds() < right.seconds());
  return seconds() < right.seconds();
}

const std::map<std::string, double> HalfLife::known_units_ = init_units();

std::map<std::string, double> HalfLife::init_units()
{
  std::map<std::string, double> result;
  result["y"] = 365. * 86400.;
  result["d"] = 86400.;
  result["h"] = 3600.;
  result["m"] = 60.;
  result["s"] = 1.0;
  result["ms"] = 1.E-3;
  result["µs"] = 1.E-6;
  result["ns"] = 1.E-9;
  result["ps"] = 1.E-12;
  result["fs"] = 1.E-15;
  result["as"] = 1.E-18;
  result["us"] = result["µs"];
  std::map<std::string, double> res2;
  for (auto r : result) {
    res2.insert(r);
    res2[boost::to_upper_copy(r.first)] = r.second;
  }
  return res2;
}

