#pragma once

#include "Record.h"

struct CommentsRecord
{
  CommentsRecord() {}
  CommentsRecord (ENSDFData& i);
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

