#include "SpinParity.h"
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"

SpinParity SpinParity::from_ensdf(std::string data)
{
  //what if tentative only parity or spin only?
  boost::trim(data);
  SpinParity ret;
  ret.parity_.from_string(data);
  ret.spins_.clear();
  boost::replace_all(data, "(", "");
  boost::replace_all(data, ")", "");
  std::vector<std::string> spin_strs;
  boost::split(spin_strs, data, boost::is_any_of(","));
  for (auto &token : spin_strs) {
    Spin spin = Spin::from_string(token);
    spin.set_quality(quality_of(data));
    ret.spins_.push_back(spin);
  }
//  if (!ret.parity_.has_quality(DataQuality::kKnown))
//    DBG << "SpinParity " << data << " --> " << ret.to_string();
  return ret;
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
