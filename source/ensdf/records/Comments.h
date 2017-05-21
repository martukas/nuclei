#pragma once

#include "Record.h"

class Translator
{
public:
 static Translator& instance()
 {
   static Translator singleton_instance;
   return singleton_instance;
 }

 std::string translate1(const std::string& s);

private:
 //singleton assurance
 Translator();
 Translator(Translator const&);
 void operator=(Translator const&);

 void make_dictionary1();

 std::list<std::pair<std::string, std::string>> dict1;
 std::list<std::pair<std::string, std::string>> dict2;
};

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
};

