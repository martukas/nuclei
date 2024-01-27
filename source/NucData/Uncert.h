#pragma once

#include <cstdint>
#include <string>

class Uncert
{
public:
  enum UncertaintyType
  {
    UndefinedType         = 0x0,
    SymmetricUncertainty  = 0x1,
    AsymmetricUncertainty = 0x2,
    LessThan              = 0x3,
    LessEqual             = 0x4,
    GreaterThan           = 0x5,
    GreaterEqual          = 0x6,
    Approximately         = 0x7,
    Calculated            = 0x8,
    Systematics           = 0x9
  };

  enum Sign
  {
    UndefinedSign         = 0x0,
    MagnitudeDefined      = 0x1,
    SignDefined           = 0x2,
    SignMagnitudeDefined  = 0x3
  };

  Uncert();
  Uncert(double d, uint16_t sigf, Sign s);
  Uncert(double d, uint16_t sigf, Sign s, double symmetricSigma);

  Uncert & operator*=(double other);
  Uncert & operator*=(const Uncert &other);
  Uncert & operator+=(const Uncert &other);
  Uncert & operator-=(const Uncert &other);
  Uncert operator*(const Uncert &other) const;
  Uncert operator+(const Uncert &other) const;
  Uncert operator-(const Uncert &other) const;
  operator double() const;

  double value() const;
  double lowerUncertainty() const;
  double upperUncertainty() const;
  UncertaintyType uncertaintyType() const;
  Sign sign() const;
  uint16_t sigfigs() const;
  uint16_t sigdec() const;

  void setValue(double val, Sign s = SignMagnitudeDefined);
  void setUncertainty(double lower, double upper, UncertaintyType type);
  void setSymmetricUncertainty(double sigma);
  void setAsymmetricUncertainty(double lowerSigma, double upperSigma);
  void setSign(Sign s);
  void setSigFigs(uint16_t sig);

  bool hasFiniteValue() const;
  bool defined() const;

  std::string to_string(bool prefix_magn, bool with_uncert = true) const;
  std::string to_markup() const; // outputs formatted text

  double val_adjusted() const;
  bool symmetric() const;
  int uncert_order() const;
  int exponent() const;
  int decimals() const;
  bool insignificant_uncert() const;
  bool zero_uncert() const;

  std::string value_str() const;
  std::string sign_prefix(bool prefix_magn) const;
  std::string uncert_str() const; // for sym and asym
  std::string sym_uncert_str() const;
  std::string asym_uncert_str() const;

private:
  double value_;
  double lower_sigma_, upper_sigma_;
  Sign sign_;
  UncertaintyType type_;
  uint16_t sigfigs_;
};
