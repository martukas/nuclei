#pragma once

#include <ensdf/records/Comments.h>
#include <ensdf/records/ProductionNorm.h>

struct NormalizationRecord
{
  NormalizationRecord() {}
  NormalizationRecord(ENSDFData& i);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  Uncert NR, NT, BR, NB, NP;

  ProdNormalizationRecord production;

  std::list<CommentsRecord> comments;
};

