#pragma once

#include "ensdf_records.h"

struct CommentsRecord
{
  CommentsRecord() {}
  CommentsRecord (size_t& idx,
                  const std::vector<std::string>& data);
  static bool match(const std::string& line, std::string rt = "");

  std::string debug() const;
  bool valid() const;

  NuclideId nuclide;
  std::string rtype;
  std::string text;

//  bool ignore {false};

  std::string extract(const std::string& line);

  static std::string translate_all(const std::string& s);
  static std::list<std::pair<std::string, std::string>> get_dictionary();
};

