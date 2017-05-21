#pragma once

#include "Comments.h"
#include <memory>

struct QValueRecord
{
  QValueRecord() {}
  QValueRecord(ENSDFData& i, bool recurse = true);
  static bool match(const std::string& line);

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  UncertainDouble Q, SN, SP, QA;
  std::string ref;

  std::list<CommentsRecord> comments;
  std::shared_ptr<QValueRecord> alternative;
};
