#include "Moment.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include "qpx_util.h"

bool Moment::valid() const
{
  return moment_.hasFiniteValue();
}

Moment Moment::from_ensdf(std::string record)
{
  std::string value_str;
  std::string uncert_str;
  std::string references_str;

  std::vector<std::string> momentparts;
  std::string rec_copy = trim_all(record);
  boost::trim(rec_copy);
  boost::split(momentparts, rec_copy, boost::is_any_of(" \r\t\n\0"));
  if (momentparts.size() >= 1)
    value_str = momentparts[0];
  if (momentparts.size() >= 2)
    uncert_str = momentparts[1];
  if (momentparts.size() >= 3)
    references_str = momentparts[2];

  if (UncertainDouble::is_uncert(value_str))
  {
    value_str = uncert_str;
    uncert_str = momentparts[0];
  }

  Moment ret;
  ret.moment_ = UncertainDouble::from_nsdf(value_str, uncert_str);

  if (!references_str.empty()) {
    boost::replace_all(references_str, "(", "");
    boost::replace_all(references_str, ")", "");
    boost::split(ret.references_, references_str, boost::is_any_of(","));
  }

  return ret;
}


const std::string Moment::to_string() const
{
  return moment_.to_string(false);
}

const std::string Moment::to_markup() const
{
  return moment_.to_markup();
}
