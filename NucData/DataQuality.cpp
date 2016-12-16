#include "DataQuality.h"
#include <boost/algorithm/string.hpp>

std::string add_qualifiers(const std::string& original,
                           const DataQuality& quality,
                           const std::string& unknown)
{
  if ( quality == DataQuality::kUnknown )
    return unknown;

  if ( quality == DataQuality::kTentative )
    return "(" + original + ")";
  if ( quality == DataQuality::kTheoretical )
    return "[" + original + "]";
  if ( quality == DataQuality::kAbout )
    return "~" + original;

  return original;
}

std::string QualifiedData::add_qualifiers(const std::string& original,
                                          const std::string& unknown) const
{
  return ::add_qualifiers(original, quality_, unknown);
}

