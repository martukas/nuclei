#pragma once

#include <util/string_extensions.h>
#include <cmath>
#include <iomanip>

inline bool is_number(std::string s)
{
  trim(s);
  if (s.empty())
    return false;
  char* p;
  strtod(s.c_str(), &p);
  return (*p == 0);
}

inline std::string to_max_precision(double number)
{
  std::stringstream ss;
  ss << std::setprecision(std::numeric_limits<double>::max_digits10) << number;
  return ss.str();
}

inline std::string to_str_precision(double number, int precision = -1)
{
  std::ostringstream ss;
  if (precision < 0)
    ss << number;
  else
    ss << std::setprecision(precision) << number;
  return ss.str();
}

inline std::string to_str_decimals(double number, int decimals = 0)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(decimals) << number;
  return ss.str();
}

struct FloatDeconstructed
{
  std::string sign;
  std::string mantissa;
  std::string exponent;

  inline void parse(std::string s)
  {
    sign.clear();
    mantissa.clear();
    exponent.clear();

    trim(s);
    if (s.empty())
      return;
    if ((s[0] == '+') || (s[0] == '-'))
    {
      sign = s[0];
      s = s.substr(1, s.size() - 1);
    }

    size_t le = s.find('e');
    size_t lE = s.find('E');
    if (le != std::string::npos)
    {
      mantissa = s.substr(0, le);
      exponent = s.substr(le + 1, s.size() - le - 1);
    }
    else if (lE != std::string::npos)
    {
      mantissa = s.substr(0, lE);
      exponent = s.substr(lE + 1, s.size() - lE - 1);
    }
    else
    {
      mantissa = s;
    }
  }

  FloatDeconstructed() {}
  FloatDeconstructed(std::string s)
  {
    parse(s);
  }
};

inline uint16_t sig_digits(std::string st)
{
  FloatDeconstructed parsed(st);
  if (parsed.mantissa.empty())
    return 0;

  //assume only one number in string
  uint16_t count = 0;
  bool past_zeros = false;
  bool had_decimal = false;
  uint16_t trailing_0s = 0;
  for (size_t i = 0; i < parsed.mantissa.size(); i++)
  {
    bool digit = std::isdigit(parsed.mantissa[i]);
    if (parsed.mantissa[i] == '.')
      had_decimal = true;
    if (digit && (parsed.mantissa[i] != '0'))
      past_zeros = true;
    if (past_zeros && digit)
      count++;
    if (past_zeros && !had_decimal && (parsed.mantissa[i] == '0'))
      trailing_0s++;
  }
  if (!had_decimal && trailing_0s)
  {
    trailing_0s = 0;
    size_t i = parsed.mantissa.size();
    while (i > 0)
    {
      i--;
      if (parsed.mantissa[i] == '0')
        trailing_0s++;
      else
        break;
    }
  }
  else
    trailing_0s = 0;

  return count - trailing_0s;
}

inline int16_t order_of(double val)
{
  if (!std::isfinite(val))
    return 0;
  else
    return std::floor(std::log10(std::abs(val)));
}

inline double get_precision(std::string value)
{
  FloatDeconstructed parsed(value);
  if (parsed.mantissa.empty())
    return 0;

  int exponent = 0;
  if (!parsed.exponent.empty() && is_number(parsed.exponent))
    exponent = std::stoi(parsed.exponent);

  int sigpos = 0;
  size_t pointpos = parsed.mantissa.find('.');
  if (pointpos != std::string::npos)
    sigpos -= parsed.mantissa.size() - pointpos - 1;

  // return factor for shifting according to exponent
  return pow(10.0, double(sigpos + exponent));
}
