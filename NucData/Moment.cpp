#include "Moment.h"

bool Moment::valid() const
{
  return moment_.hasFiniteValue();
}

const std::string Moment::to_string() const
{
  return moment_.to_string(false);
}

const std::string Moment::to_markup() const
{
  return moment_.to_markup();
}
