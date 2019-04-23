#include <NucData/DecayInfo.h>
#include <util/logger.h>
#include "qpx_util.h"
#include <boost/regex.hpp>

#include <ensdf/Fields.h>

bool DecayInfo::valid() const
{
  return parent.valid() && mode.valid();
}

std::string DecayInfo::to_string() const
{
  std::string ret = parent.symbolicName() + " " + mode.to_string();
//  ret += " block=" + std::to_string(block.first) + "-" + std::to_string(block.second)
  if (hl.valid())
    ret += " " + hl.preferred_units().to_string(true);
  return ret;
}

std::string DecayInfo::name() const
{
  return mode.to_string();
}


