#include <NucData/Parity.h>

Parity::Parity()
{
  quality_ = DataQuality::kUnknown;
}

Parity::Parity(const Parity &other)
  : QualifiedData(other)
  , parity_(other.parity_)
{}

Parity::Parity(const EnumParity& p, const DataQuality& q)
{
  parity_ = p;
  quality_ = q;
}

bool Parity::valid() const
{
  return ( quality_ != DataQuality::kUnknown );
}

const std::string Parity::to_string() const
{
  if ( quality_ == DataQuality::kUnknown )
    return "";
  else if ( parity_ == EnumParity::kPlus )
    return "+";
  else if ( parity_ == EnumParity::kMinus)
    return "-";
  else
    return "\u00B1";
}

const std::string Parity::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}


bool operator==(const Parity &left, const Parity &right)
{
  return (left.parity_ == right.parity_);
}

bool operator<(const Parity &left, const Parity &right)
{
  return (left.parity_ < right.parity_);
}

bool operator>(const Parity &left, const Parity &right)
{
  return (left.parity_ > right.parity_);
}
