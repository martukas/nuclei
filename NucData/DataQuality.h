#pragma once

#include <string>

enum class DataQuality
{
  kKnown = 0,
  kUnknown = 1,
  kTentative = 2,
  kTheoretical = 3,
  kAbout = 4
};

std::string add_qualifiers(const std::string& original,
                           const DataQuality& quality,
                           const std::string& unknown = "?");

class QualifiedData
{
public:
  QualifiedData() {}
  QualifiedData(const QualifiedData &other);

  void set_quality(const DataQuality& q);
  DataQuality quality() const;

protected:
  DataQuality quality_ {DataQuality::kKnown};

  std::string add_qualifiers(const std::string& original,
                             const std::string& unknown = "?") const;
};
