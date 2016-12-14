#ifndef GW_DataQuality_H
#define GW_DataQuality_H

#include <string>


enum class DataQuality { kKnown = 0, kUnknown = 1, kTentative = 2, kTheoretical = 3, kAbout = 4 } ;

DataQuality quality_of(const std::string&);
std::string strip_qualifiers(const std::string& original);
std::string add_qualifiers(const std::string& original,
                           const DataQuality& quality,
                           const std::string& unknown = "?");


class QualifiedData
{
public:
  QualifiedData() {}

  QualifiedData(const QualifiedData &other)
    : quality_(other.quality_)
  {}

  void set_quality(const DataQuality& q) { quality_ = q; }
  DataQuality quality() const { return quality_; }


protected:
  DataQuality quality_ {DataQuality::kKnown};

  std::string add_qualifiers(const std::string& original,
                             const std::string& unknown = "?") const;
};

#endif
