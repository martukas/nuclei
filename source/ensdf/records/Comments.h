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
 std::string to_html(std::string s);

private:
 //singleton assurance
 Translator();
 Translator(Translator const&);
 void operator=(Translator const&);

 void make_dictionary1();
 void make_dictionary2();

 void add_dict2(char c, std::string alt1, std::string alt2);

 std::list<std::pair<std::string, std::string>> dict1;

 std::map<char, std::string> dict2c1;
 std::map<char, std::string> dict2c2;
 std::map<std::string, std::string> dict2_s;
};

struct CommentsRecord
{
  CommentsRecord() {}
  CommentsRecord (ENSDFData& i);
  static bool match(const std::string& line, std::string rt = "");

  std::string debug() const;
  std::string html() const;
  bool valid() const;

  NuclideId nuclide;
  std::string rtype;
  std::string text;

//  bool ignore {false};

  std::string extract(const std::string& line);

  std::string adjust_case(const std::string& line);
};

