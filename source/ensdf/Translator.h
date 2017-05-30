#pragma once

#include <string>
#include <map>
#include <list>

class Translator
{
public:
 static Translator& instance()
 {
   static Translator singleton_instance;
   return singleton_instance;
 }

 std::string auth_capitalize(const std::string& s);
 void to_camel(std::string& s);

 std::string translate1(const std::string& s);
 std::string to_html(std::string s);

 void spaces_to_html(std::string& s);
 std::string spaces_to_html_copy(const std::string& s);

 std::string hist_key(const std::string& s);
 std::string hist_eval_type(const std::string& s);

private:
 //singleton assurance
 Translator();
 Translator(Translator const&);
 void operator=(Translator const&);

 void make_dictionary1();
 void make_dictionary2();
 void make_hist();

 void add_dict2(char c, std::string alt1, std::string alt2);

 std::list<std::pair<std::string, std::string>> dict1;

 std::map<char, std::string> dict2c1;
 std::map<char, std::string> dict2c2;
 std::map<std::string, std::string> dict2_s;

 std::map<std::string, std::string> hist_keys_;
 std::map<std::string, std::string> hist_eval_types_;

};

