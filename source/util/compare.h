#pragma once

#include <cmath>

// \todo min/max may exist in std for newer compilers

inline double min(double a, double b, double c)
{
  return std::min(a, std::min(b, c));
}

inline double max(double a, double b, double c)
{
  return std::max(a, std::max(b, c));
}

inline double mid(double a, double b, double c)
{
  // Compare each three number to find middle
  // number. Enter only if a > b
  if (a > b)
  {
    if (b > c)
      return b;
    else if (a > c)
      return c;
    else
      return a;
  }
  else
  {
    // Decided a is not greater than b.
    if (a > c)
      return a;
    else if (b > c)
      return c;
    else
      return b;
  }
}
