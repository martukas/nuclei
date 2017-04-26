#include "SpinParity.h"

void SpinParity::set_parity(Parity p)
{
  parity_ = p;
}

void SpinParity::add_spin(Spin s)
{
  spins_.push_back(s);
}

bool SpinParity::valid() const
{
  return ( (parity_.quality() != DataQuality::kUnknown) && (spins_.size() == 1) );
}

std::string SpinParity::to_string() const
{
  std::string ret;
  for (size_t i=0; i < spins_.size(); ++i) {
    ret += spins_[i].to_string() + parity_.to_string();
    ret += ((i+1) < spins_.size()) ? "," : "";
  }
  return add_qualifiers(ret, parity_.quality());
}
