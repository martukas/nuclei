#include "DataQuality.h"

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

QualifiedData::QualifiedData(const QualifiedData &other)
  : quality_(other.quality_)
{

}

void QualifiedData::set_quality(const DataQuality& q)
{
  quality_ = q;
}

DataQuality QualifiedData::quality() const
{
  return quality_;
}

std::string QualifiedData::add_qualifiers(const std::string& original,
                                          const std::string& unknown) const
{
  return ::add_qualifiers(original, quality_, unknown);
}

std::string QualifiedData::debug() const
{
  return add_qualifiers("");
}


