#pragma once

#include "ensdf_records.h"
#include "id_record.h"

struct CommentsRecord
{
  static bool is(const std::string& line);

  static CommentsRecord parse(size_t& idx,
                              const std::vector<std::string>& data);

  static CommentsRecord from_id(const IdRecord &record,
                                BlockIndices block);

  std::string debug() const;

  NuclideId nuclide;
  BlockIndices block;
  std::string rtype;
  std::string ctype;
  std::string text;
  bool translate {false};

  static std::string translate_all(const std::string& s);

  static std::multimap<size_t, std::pair<std::string, std::string>> get_dictionary();
};

