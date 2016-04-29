#include "SpinParity.h"
#include <boost/algorithm/string.hpp>
#include "custom_logger.h"

SpinParity::SpinParity()
{
}


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
    Spin spin;
    spin.from_string(token);
    spin.set_quality(ret.parity_.quality(data));
    ret.spins_.push_back(spin);
  }
//  if (!ret.parity_.is_quality(DataQuality::kKnown))
//    DBG << "SpinParity " << data << " --> " << ret.to_string();
  return ret;
}


bool SpinParity::valid() const
{
  return ( !parity_.is_quality(DataQuality::kUnknown) && (spins_.size() == 1) );
}

int SpinParity::doubled_spin() const
{
  if (spins_.size() < 1)
    return 0;
  else
    return spins_[0].doubled_spin();
}

std::string SpinParity::to_string() const
{
  std::string ret;
  for (int i=0; i < spins_.size(); ++i) {
    ret += spins_[i].to_string() + parity_.to_string();
    ret += ((i+1) < spins_.size()) ? "," : "";
  }
  ret = parity_.add_qualifiers(ret);
  return ret;
}
