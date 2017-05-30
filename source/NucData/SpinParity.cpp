#include "SpinParity.h"
#include <boost/algorithm/string.hpp>

void SpinParity::set_parity(Parity p)
{
  parity_ = p;
}

void SpinParity::add_spin(Spin s)
{
  spins_.push_back(s);
  if (s.valid())
    have_valid_spins_ = true;
  qs_.insert(s.quality());
}

bool SpinParity::valid() const
{
  return ( (parity_.quality() != DataQuality::kUnknown) || have_valid_spins_ );
}

std::string SpinParity::to_string() const
{
  std::list<std::string> spinstrs;
  for (auto s : spins_)
    spinstrs.push_back(s.to_string());
  std::string ret = boost::join(spinstrs, ",");
  return add_qualifiers(ret, parity_.quality());
}

//bool operator<(const SpinParity &left, const SpinParity &right)
//{
//  return (left.parity_ < right.parity_);
//}

//bool operator>(const SpinParity &left, const SpinParity &right)
//{

//}

//bool operator==(const SpinParity &left, const SpinParity &right)
//{

//}
