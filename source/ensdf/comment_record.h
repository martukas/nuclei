#pragma once

#include "ensdf_records.h"
#include "id_record.h"

struct CommentsRecord
{
  CommentsRecord() {}
  CommentsRecord (size_t& idx,
                  const std::vector<std::string>& data);
  static bool match(const std::string& line, std::string rt = "");

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  BlockIndices block;
  std::string rtype;
  std::string ctype;
  std::string text;
  bool translate {false};

  static std::string translate_all(const std::string& s);
  static std::multimap<size_t, std::pair<std::string, std::string>> get_dictionary();
};

