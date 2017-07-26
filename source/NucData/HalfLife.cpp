#include "HalfLife.h"

#include "qpx_util.h"

HalfLife::HalfLife()
  : HalfLife(kDoubleNaN, false, "")
{}

HalfLife::HalfLife(double val, bool approx, std::string units)
  : HalfLife(Uncert(val, order_of(val), Uncert::SignMagnitudeDefined),
             approx, units)
{
}


HalfLife::HalfLife(Uncert time, bool approx, std::string units)
  : time_(time)
  , units_(units)
  , tentative_(approx)
{
//  time_.setUncertainty(time.lowerUncertainty(),
//                       time.upperUncertainty(),
//                       Uncert::SymmetricUncertainty);
}

Uncert HalfLife::time() const
{
  return time_;
}

std::string HalfLife::units() const
{
  return units_;
}

bool HalfLife::tentative() const
{
  return tentative_;
}

HalfLife HalfLife::preferred_units() const
{
  auto ret = *this;
  if (e_units_.count(units_))
  {
    ret.time_ *= e_units_.at(units_);
    ret.units_ = preferred_e_units(ret.time_.value());
    ret.time_ *= 1.0 / e_units_.at(ret.units_);
  }
  if (time_units_.count(units_))
  {
    ret.time_ *= time_units_.at(units_);
    ret.units_ = preferred_time_units(ret.time_.value());
    ret.time_ *= 1.0 / time_units_.at(ret.units_);
  }
  return ret;
}

bool HalfLife::valid() const
{
  //  INFO << "HL " << to_string() << " valid=" << !boost::math::isnan(time_.value());
  return !boost::math::isnan(time_.value());
}

double HalfLife::ev() const
{
  if (!e_units_.count(units_))
    return kDoubleNaN;
  Uncert tmp = time_;
  tmp *= e_units_.at(units_);
  //  INFO << "HL " << to_string() << " s=" << tmp.value();
  return tmp.value();
}

double HalfLife::seconds() const
{
  if (!time_units_.count(units_))
    return kDoubleNaN;
  Uncert tmp = time_;
  tmp *= time_units_.at(units_);
  //  INFO << "HL " << to_string() << " s=" << tmp.value();
  return tmp.value();
}

bool HalfLife::stable() const
{
  //  INFO << "HL " << to_string() << " stable=" << boost::math::isinf(time_.value());
  return boost::math::isinf(time_.value());
}

std::string HalfLife::to_string(bool with_uncert) const
{
  if (!valid())
    return "";
  std::string ret;
  if (stable())
    ret = "stable";
  else
    ret = time_.to_string(false, with_uncert);
  if (units_.size())
    ret += " " + units_;
  if (tentative_)
    return "(" + ret + ")";
  return ret;
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




std::string HalfLife::preferred_time_units(double secs)
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
  return "s";
}

std::string HalfLife::preferred_e_units(double ev)
{
  if (ev > 1.E6)
    return "MeV";
  else if (ev > 1.E3)
    return "keV";
  return "eV";
}

const std::map<std::string, double> HalfLife::time_units_ = init_time_units();
const std::map<std::string, double> HalfLife::e_units_ = init_e_units();

std::map<std::string, double> HalfLife::init_time_units()
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
  for (auto r : result)
  {
    res2.insert(r);
    res2[boost::to_upper_copy(r.first)] = r.second;
  }
  return res2;
}

std::map<std::string, double> HalfLife::init_e_units()
{
  std::map<std::string, double> result;

  result["eV"] = 1.0;
  result["keV"] = 1.E3;
  result["MeV"] = 1.E6;

  std::map<std::string, double> res2;
  for (auto r : result)
  {
    res2.insert(r);
    res2[boost::to_upper_copy(r.first)] = r.second;
  }
  return res2;
}

