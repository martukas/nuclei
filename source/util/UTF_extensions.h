#pragma once

#include <string>
#include <vector>
#include <cmath>

const std::vector<std::string> k_UTF_superscripts =
{
  "\u2070", "\u00B9", "\u00B2",
  "\u00B3", "\u2074", "\u2075",
  "\u2076", "\u2077", "\u2078",
  "\u2079"
};

const std::vector<std::string> k_UTF_subscripts =
{
  "\u2080", "\u2081", "\u2082",
  "\u2083", "\u2084", "\u2085",
  "\u2086", "\u2087", "\u2088",
  "\u2089"
};

inline std::string UTF_superscript(int32_t value)
{
  if (value < 0)
    return "\u207B" + UTF_superscript(-value);
  else if (value < 10)
    return k_UTF_superscripts[value];
  else
    return UTF_superscript(value / 10) + UTF_superscript(value % 10);
}

inline std::string UTF_subscript(int32_t value)
{
  if (value < 0)
    return "\u208B" + UTF_subscript(-value);
  else if (value < 10)
    return k_UTF_subscripts[value];
  else
    return UTF_subscript(value / 10) + UTF_subscript(value % 10);
}

inline std::string UTF_superscript_dbl(double value, uint16_t decimals)
{
  std::string sign = (value < 0) ? "\u207B" : "";
  value = std::abs(value);
  uint32_t upper = std::floor(value);
  std::string result = UTF_superscript(upper);
  if (decimals) {
    uint32_t lower = std::round((value - upper) * pow(10.0, decimals));
    if (lower)
      result += "\u00B7" + UTF_superscript(lower);
  }
  if (result != "\u2070")
    return sign + result;
  else
    return result;
}

inline std::string UTF_subscript_dbl(double value, uint16_t decimals)
{
  std::string sign = (value < 0) ? "\u208B" : "";
  value = std::abs(value);
  uint32_t upper = std::floor(value);
  std::string result = UTF_subscript(upper);
  if (decimals) {
    uint32_t lower = std::round((value - upper) * pow(10.0, decimals));
    if (lower)
      result += "." + UTF_subscript(lower);
  }
  if (result != "\u2080")
    return sign + result;
  else
    return result;
}
