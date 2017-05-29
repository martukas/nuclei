#include "Uncert.h"

#include <limits>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "qpx_util.h"
//#include "custom_logger.h"

Uncert::Uncert()
  : value_(std::numeric_limits<double>::quiet_NaN())
  , lower_sigma_(std::numeric_limits<double>::quiet_NaN())
  , upper_sigma_(std::numeric_limits<double>::quiet_NaN())
  , sign_(UndefinedSign)
  , type_(UndefinedType)
  , sigfigs_(0)
{
}

Uncert::Uncert(double d, uint16_t sigf, Sign s)
  : value_(d)
  , lower_sigma_(0.0)
  , upper_sigma_(0.0)
  , sign_(s)
  , type_(SymmetricUncertainty)
  , sigfigs_(sigf)
{
}

Uncert::Uncert(double d, uint16_t sigf, Uncert::Sign s, double symmetricSigma)
  : value_(d)
  , lower_sigma_(symmetricSigma)
  , upper_sigma_(symmetricSigma)
  , sign_(s)
  , type_(SymmetricUncertainty)
  , sigfigs_(sigf)
{
}

Uncert &Uncert::operator =(const Uncert &other)
{
  if (this != &other) {
    value_ = other.value_;
    lower_sigma_ = other.lower_sigma_;
    upper_sigma_ = other.upper_sigma_;
    sign_ = other.sign_;
    type_ = other.type_;
    sigfigs_ = other.sigfigs_;
  }
  return *this;
}

double Uncert::value() const
{
  return value_;
}

double Uncert::lowerUncertainty() const
{
  return lower_sigma_;
}

double Uncert::upperUncertainty() const
{
  return upper_sigma_;
}

Uncert::UncertaintyType Uncert::uncertaintyType() const
{
  return type_;
}

Uncert::Sign Uncert::sign() const
{
  return sign_;
}

uint16_t Uncert::sigfigs() const
{
  return sigfigs_;
}

uint16_t Uncert::sigdec() const
{
  int orderOfValue = order_of(value_);
  if (sigfigs_ > orderOfValue) {
    return (sigfigs_ - orderOfValue - 1);
  }
  else
    return 0;
}

void Uncert::setValue(double val, Sign s)
{
  value_ = val;
  sign_ = s;
}

void Uncert::setUncertainty(double lower, double upper, Uncert::UncertaintyType type)
{
  lower_sigma_ = lower;
  upper_sigma_ = upper;
  type_ = type;
}

void Uncert::setSymmetricUncertainty(double sigma)
{
  setUncertainty(sigma, sigma, SymmetricUncertainty);
}

void Uncert::setAsymmetricUncertainty(double lowerSigma, double upperSigma)
{
  setUncertainty(lowerSigma, upperSigma, AsymmetricUncertainty);
}

void Uncert::setSign(Uncert::Sign s)
{
  sign_ = s;
}

void Uncert::setSigFigs(uint16_t sig)
{
  sigfigs_ = sig;
}

bool Uncert::hasFiniteValue() const
{
  if (sign() != Uncert::MagnitudeDefined &&
      sign() != Uncert::SignMagnitudeDefined)
    return false;
  
  if (uncertaintyType() == Uncert::SymmetricUncertainty ||
      uncertaintyType() == Uncert::AsymmetricUncertainty ||
      uncertaintyType() == Uncert::Approximately ||
      uncertaintyType() == Uncert::Calculated ||
      uncertaintyType() == Uncert::Systematics)
    return true;

  return false;
}

bool Uncert::defined() const
{
  return (sign() != UndefinedSign) &&
      (uncertaintyType() != UndefinedType);
}

std::string Uncert::to_string(bool prefix_magn, bool with_uncert) const
{
  std::string plusminus("\u00B1");
  std::string times_ten("\u00D710");

  std::string signprefix;
  double val = value_;
  if (sign_ == MagnitudeDefined) {
    if (prefix_magn)
      signprefix = plusminus;
    val = std::abs(val);
  }
  else if (sign_ == UndefinedSign) {
    signprefix = "?";
    val = std::abs(val);
  }
  
  switch (type_) {
  case Systematics:
    return signprefix + to_str_precision(val) + " (sys)";
  case Calculated:
    return signprefix + to_str_precision(val) + " (calc)";
  case Approximately:
    return ( (value_ != 0) ? "~" : "") + signprefix + to_str_precision(val);
  case GreaterEqual:
    return "≥" + signprefix + to_str_precision(val);
  case GreaterThan:
    return ">" + signprefix + to_str_precision(val);
  case LessEqual:
    return "≤" + signprefix + to_str_precision(val);
  case LessThan:
    return "<" + signprefix + to_str_precision(val);
  case SymmetricUncertainty:
  case AsymmetricUncertainty:
  {
    // return precise numbers without uncertainty
    if (upper_sigma_ == 0.0 && lower_sigma_ == 0.0)
      return signprefix + to_str_precision(val);

    bool symmetric = (type_ == SymmetricUncertainty) || (upper_sigma_ == lower_sigma_);
    
    int orderOfValue = order_of(val);
    int orderOfUncert;

    if (symmetric)
      orderOfUncert = order_of(lower_sigma_);
    else
      orderOfUncert = std::max(order_of(lower_sigma_), order_of(upper_sigma_));

    bool insiginficant = (orderOfValue - orderOfUncert) > 6;
    int targetOrder = orderOfValue;
    if (orderOfUncert > orderOfValue)
      targetOrder = orderOfUncert;

    int exponent = 0;
    if ((targetOrder > 4) || (targetOrder < -3))
      exponent = targetOrder;

    int decimals = 0;
    if (sigfigs_ > (orderOfValue - exponent))
      decimals = sigfigs_ - (orderOfValue - exponent) - 1;

    std::string val_str = to_str_decimals(value_ / pow(10.0, exponent), decimals);

    std::string uncertstr;
    if (!insiginficant) {
      if (symmetric) //symmetrical
      {
        double unc_shifted = lower_sigma_ / pow(10.0, exponent);
        std::string unc_str;
        if (!decimals)
          unc_str = to_str_precision(unc_shifted);
        else if (unc_shifted < 1.0)
          unc_str = to_str_precision(unc_shifted / pow(10.0, -decimals));
        else
          unc_str = to_str_decimals(unc_shifted, orderOfUncert - exponent + decimals );
        if (!unc_str.empty())
          uncertstr = plusminus + unc_str;
      }
      else //asymmetrical
      {
        double lower_shifted = lower_sigma_ / pow(10.0, exponent);
        double upper_shifted = upper_sigma_ / pow(10.0, exponent);
        std::string unc_str_l, unc_str_u;
        if (decimals == 0) {
          unc_str_l = UTF_subscript(lower_shifted);
          unc_str_u = UTF_superscript(upper_shifted);
        } else {
          bool unc_move_decimal = true;
          if (!symmetric && ((lower_shifted >= 1.0) || (upper_shifted >= 1.0)))
            unc_move_decimal = false;
          else if (symmetric &&
                   (lower_shifted >= 1.0))
            unc_move_decimal = false;
          if (unc_move_decimal) {
            unc_str_l = UTF_subscript(lower_shifted / pow(10.0, -decimals)); // to_str_precision(lower_shifted / pow(10.0, -decimals));
            unc_str_u = UTF_superscript(upper_shifted / pow(10.0, -decimals)); //to_str_precision(upper_shifted / pow(10.0, -decimals));
          } else {
            unc_str_l = UTF_subscript_dbl(lower_shifted,
                                          orderOfUncert - exponent + decimals ); //to_str_decimals(lower_shifted, uncprecision );
            unc_str_u = UTF_superscript_dbl(upper_shifted,
                                            orderOfUncert - exponent + decimals );
          }
        }

        if (!unc_str_u.empty())
          uncertstr += "\u207A" + unc_str_u;
        if (!unc_str_l.empty())
          uncertstr += "\u208B" + unc_str_l;
      }
    } else if (value_ != 0)
      uncertstr = "~";

    std::string result = val_str;

    if (with_uncert)
     result += uncertstr;

    if (exponent)
      result = "(" + result + ")" + times_ten + UTF_superscript(exponent);

    //    DBG << "(" << value_ << "-" << lower_sigma_ << "+" << upper_sigma_ << ") = "
    //        << signprefix << result;

    return signprefix + result;
  }
  default:
    return "undefined";
  }
}

std::string Uncert::to_markup() const
{
  std::string ret = to_string(true);
  boost::replace_all(ret, "(sys)", "<i>(sys)</i>");
  boost::replace_all(ret, "(calc)", "<i>(calc)</i>");
  boost::replace_all(ret, "<", "&lt;");
  boost::replace_all(ret, ">", "&gt;");
  return ret;
}

Uncert & Uncert::operator*=(double other)
{
  setValue(value() * other);
  if (other >= 0.0 ||
      uncertaintyType() == SymmetricUncertainty ||
      uncertaintyType() == Approximately ||
      uncertaintyType() == Calculated ||
      uncertaintyType() == Systematics
      ) {
    setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, uncertaintyType());
  }
  else { // "other" is always negative in this branch!
    if (uncertaintyType() == AsymmetricUncertainty)
      setUncertainty(upperUncertainty() * other, lowerUncertainty() * other, uncertaintyType());
    else if (uncertaintyType() == LessThan)
      setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, GreaterThan);
    else if (uncertaintyType() == LessEqual)
      setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, GreaterEqual);
    else if (uncertaintyType() == GreaterThan)
      setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, LessThan);
    else if (uncertaintyType() == GreaterEqual)
      setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, LessEqual);
    else
      setUncertainty(lowerUncertainty() * other, upperUncertainty() * other, uncertaintyType());
  }
  return *this;
}

Uncert & Uncert::operator*=(const Uncert &other)
{
  //Chaltura!!!
  *this *= other.value();
  return *this;
}

Uncert &Uncert::operator +=(const Uncert &other)
{
  uint16_t sd1 = sigdec();
  uint16_t sd2 = other.sigdec();
  setValue(value() + other.value());
  setSigFigs(std::min(sd1, sd2) + order_of(value_) + 1);
  if (uncertaintyType() == other.uncertaintyType())
    setUncertainty(lowerUncertainty() + other.lowerUncertainty(), upperUncertainty() + other.upperUncertainty(), uncertaintyType());
  else
    setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UndefinedType);
  return *this;
}

Uncert &Uncert::operator -=(const Uncert &other)
{
  uint16_t sd1 = sigdec();
  uint16_t sd2 = other.sigdec();
  setValue(value() - other.value());
  setSigFigs(std::min(sd1, sd2) + order_of(value_) + 1);
  if (uncertaintyType() == other.uncertaintyType())
    setUncertainty(lowerUncertainty() + other.lowerUncertainty(), upperUncertainty() + other.upperUncertainty(), uncertaintyType());
  else
    setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UndefinedType);
  return *this;
}

Uncert Uncert::operator +(const Uncert &other) const
{
  Uncert result(*this);
  result += other;
  return result;
}

Uncert Uncert::operator -(const Uncert &other) const
{
  Uncert result(*this);
  result -= other;
  return result;
}

Uncert Uncert::operator *(const Uncert &other) const
{
  Uncert result(*this);
  result *= other;
  return result;
}

Uncert::operator double() const
{
  return value_;
}
