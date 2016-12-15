#include "UncertainDouble.h"

#include <limits>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "qpx_util.h"
#include "custom_logger.h"

UncertainDouble::UncertainDouble()
  : value_(std::numeric_limits<double>::quiet_NaN())
  , lower_sigma_(std::numeric_limits<double>::quiet_NaN())
  , upper_sigma_(std::numeric_limits<double>::quiet_NaN())
  , sign_(UndefinedSign)
  , type_(UndefinedType)
  , sigfigs_(0)
{
}

UncertainDouble::UncertainDouble(double d, uint16_t sigf, Sign s)
  : value_(d)
  , lower_sigma_(0.0)
  , upper_sigma_(0.0)
  , sign_(s)
  , type_(SymmetricUncertainty)
  , sigfigs_(sigf)
{
}

UncertainDouble::UncertainDouble(double d, uint16_t sigf, UncertainDouble::Sign s, double symmetricSigma)
  : value_(d)
  , lower_sigma_(symmetricSigma)
  , upper_sigma_(symmetricSigma)
  , sign_(s)
  , type_(SymmetricUncertainty)
  , sigfigs_(sigf)
{
}

UncertainDouble &UncertainDouble::operator =(const UncertainDouble &other)
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

double UncertainDouble::value() const
{
  return value_;
}

double UncertainDouble::lowerUncertainty() const
{
  return lower_sigma_;
}

double UncertainDouble::upperUncertainty() const
{
  return upper_sigma_;
}

UncertainDouble::UncertaintyType UncertainDouble::uncertaintyType() const
{
  return type_;
}

UncertainDouble::Sign UncertainDouble::sign() const
{
  return sign_;
}

uint16_t UncertainDouble::sigfigs() const
{
  return sigfigs_;
}

uint16_t UncertainDouble::sigdec() const
{
  int orderOfValue = order_of(value_);
  if (sigfigs_ > orderOfValue) {
    return (sigfigs_ - orderOfValue - 1);
  }
  else
    return 0;
}

void UncertainDouble::setValue(double val, Sign s)
{
  value_ = val;
  sign_ = s;
}

void UncertainDouble::setUncertainty(double lower, double upper, UncertainDouble::UncertaintyType type)
{
  lower_sigma_ = lower;
  upper_sigma_ = upper;
  type_ = type;
}

void UncertainDouble::setSymmetricUncertainty(double sigma)
{
  setUncertainty(sigma, sigma, SymmetricUncertainty);
}

void UncertainDouble::setAsymmetricUncertainty(double lowerSigma, double upperSigma)
{
  setUncertainty(lowerSigma, upperSigma, AsymmetricUncertainty);
}

void UncertainDouble::setSign(UncertainDouble::Sign s)
{
  sign_ = s;
}

void UncertainDouble::setSigFigs(uint16_t sig)
{
  sigfigs_ = sig;
}

bool UncertainDouble::hasFiniteValue() const
{
  if (    sign() != UncertainDouble::MagnitudeDefined &&
          sign() != UncertainDouble::SignMagnitudeDefined )
    return false;
  
  if (    uncertaintyType() == UncertainDouble::SymmetricUncertainty ||
          uncertaintyType() == UncertainDouble::AsymmetricUncertainty ||
          uncertaintyType() == UncertainDouble::Approximately ||
          uncertaintyType() == UncertainDouble::Calculated ||
          uncertaintyType() == UncertainDouble::Systematics )
    return true;
  return false;
}

bool UncertainDouble::is_uncert(std::string str)
{
  return (str == "LT" ||
          str == "GT" ||
          str == "LE" ||
          str == "GE" ||
          str == "AP" ||
          str == "CA" ||
          str == "SY");
}


UncertainDouble UncertainDouble::from_nsdf(std::string val, std::string uncert)
{
  boost::trim(val);
  boost::trim(uncert);

  bool flag_tentative = false;
  bool flag_theoretical = false;
  if (boost::contains(val, "(") || boost::contains(val, ")")) //what if sign only?
  {
    boost::replace_all(val, "(", "");
    boost::replace_all(val, ")", "");
    flag_tentative = true;
  }
  else if (boost::contains(val, "[") || boost::contains(val, "]")) //what if sign only?
  {
    boost::replace_all(val, "[", "");
    boost::replace_all(val, "]", "");
    flag_theoretical = true;
  }

  if (val.empty() || !is_number(val))
    return UncertainDouble();

  UncertainDouble result(boost::lexical_cast<double>(val), sig_digits(val), UncertainDouble::UndefinedSign);
  double val_order = get_precision(val);

  if (boost::contains(val, "+") || boost::contains(val, "-"))
    result.setSign(UncertainDouble::SignMagnitudeDefined);
  else
    result.setSign(UncertainDouble::MagnitudeDefined);

  // parse uncertainty
  // symmetric or special case (consider symmetric if not + and - are both contained in string)
  if ( !( boost::contains(uncert,"+") && boost::contains(uncert, "-"))
       || flag_tentative ) {
    if (uncert == "LT")
      result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, UncertainDouble::LessThan);
    else if (uncert == "GT")
      result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), UncertainDouble::GreaterThan);
    else if (uncert == "LE")
      result.setUncertainty(-std::numeric_limits<double>::infinity(), 0.0, UncertainDouble::LessEqual);
    else if (uncert == "GE")
      result.setUncertainty(0.0, std::numeric_limits<double>::infinity(), UncertainDouble::GreaterEqual);
    else if (uncert == "AP" || uncert.empty() || !is_number(uncert) || flag_tentative)
      result.setUncertainty(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), UncertainDouble::Approximately);
    else if (uncert == "CA" || flag_theoretical)
      result.setUncertainty(0.0, 0.0, UncertainDouble::Calculated);
    else if (uncert == "SY")
      result.setUncertainty(0.0, 0.0, UncertainDouble::Systematics);
    else {
      // determine significant figure
      if (!uncert.empty() && is_number(uncert))
        result.setSymmetricUncertainty(val_order * boost::lexical_cast<int16_t>(uncert));
      else
        result.setUncertainty(std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::quiet_NaN(),
                              UncertainDouble::UndefinedType);
    }
  }
  // asymmetric case
  else {
    bool inv = false;
    std::string uposstr, unegstr;
    boost::regex expr{"^\\+([0-9]+)\\-([0-9]+)$"};
    boost::regex inv_expr{"^\\-([0-9]+)\\+([0-9]+)$"};
    boost::smatch what;
    if (boost::regex_match(uncert, what, expr) && (what.size() == 3))
    {
      uposstr = what[1];
      unegstr = what[2];
    }
    else if (boost::regex_match(uncert, what, inv_expr) && (what.size() == 3))
    {
      unegstr = what[1];
      uposstr = what[2];
      inv = true;
    }

    uint16_t upositive = 0;
    uint16_t unegative = 0;

    boost::trim(uposstr);
    boost::trim(unegstr);

    if (!uposstr.empty() && is_number(uposstr))
      upositive = boost::lexical_cast<int16_t>(uposstr);

    if (!unegstr.empty() && is_number(unegstr))
      unegative = boost::lexical_cast<int16_t>(unegstr);

    if (inv)
      DBG << "Inverse asymmetric uncert " << uncert << " expr-> " << uposstr << "," << unegstr
          << " parsed as " << upositive << "," << unegative;

    // work aournd bad entries with asymmetric uncertainty values of 0.
    if (upositive == 0.0 || unegative == 0.0)
    {
      result.setUncertainty(std::numeric_limits<double>::quiet_NaN(),
                            std::numeric_limits<double>::quiet_NaN(),
                            UncertainDouble::Approximately);
      WARN << "Found asymmetric error of 0 in '"
           << uncert << "'. Auto-changing to 'approximately'";
    }
    else
      result.setAsymmetricUncertainty(val_order * unegative,
                                      val_order * upositive);
  }

//  if (result.type_ == AsymmetricUncertainty)
//    DBG << std::setw(8) << val << std::setw(7) << uncert
//        << " finite=" << result.hasFiniteValue()
//        << " has " << result.sigfigs() << " sigfigs " << " order " << val_order
//        << " parsed as " << result.value_ << "+" << result.upper_sigma_ << "-" << result.lower_sigma_
//        << " renders " << result.to_string(false)
//           ;

  return result;
}


std::string UncertainDouble::to_string(bool prefix_magn, bool with_uncert) const
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
    return "~" + signprefix + to_str_precision(val);
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
    bool as_percent = (orderOfValue - orderOfUncert) > 6; //not implemented
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
    } else
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

std::string UncertainDouble::to_markup() const
{
  std::string ret = to_string(true);
  boost::replace_all(ret, "(sys)", "<i>(sys)</i>");
  boost::replace_all(ret, "(calc)", "<i>(calc)</i>");
  boost::replace_all(ret, "<", "&lt;");
  boost::replace_all(ret, ">", "&gt;");
  return ret;
}

UncertainDouble & UncertainDouble::operator*=(double other)
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

UncertainDouble &UncertainDouble::operator +=(const UncertainDouble &other)
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

UncertainDouble UncertainDouble::operator +(const UncertainDouble &other) const
{
  UncertainDouble result(*this);
  result += other;
  return result;
}

UncertainDouble::operator double() const
{
  return value_;
}
