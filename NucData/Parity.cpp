#include "Parity.h"

const std::string Parity::to_string() const
{
  if ( quality_ == DataQuality::kUnknown )
    return "";
  else if ( parity_ == EnumParity::kPlus )
    return "+";
  else
    return "-";
}

const std::string Parity::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}


bool operator==(const Parity &left, const Parity &right)
{
  return (left.parity_ == right.parity_);
}
