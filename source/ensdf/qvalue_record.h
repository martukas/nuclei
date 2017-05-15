#pragma once

#include "ensdf_records.h"
#include "comment_record.h"
#include <memory>

struct QValueRecord
{
  static bool is(const std::string& line);
  static QValueRecord parse(size_t& idx,
                            const std::vector<std::string>& data,
                            bool recurse = true);

  std::string debug() const;

  NuclideId nuc_id;
  UncertainDouble Q, SN, SP, QA;
  std::string ref;

  std::list<CommentsRecord> comments;
  std::shared_ptr<QValueRecord> alternative;
};
