#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

// trim from start (in place)
static inline void ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
  {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
static inline void rtrim(std::string& s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
  {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s)
{
  ltrim(s);
  rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s)
{
  ltrim(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s)
{
  rtrim(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s)
{
  trim(s);
  return s;
}

inline std::string trim_all(std::string text)
{
  std::istringstream iss(text);
  text = "";
  std::string s;
  while (iss >> s)
  {
    if (text != "")
      text += " " + s;
    else text = s;
  }
  return text;
}

//template<typename T>
//inline std::string join(T strings, std::string delimiter)
//{
//  std::string ret;
//  for (const auto& s : strings)
//  {
//    if (!ret.empty())
//      ret += delimiter;
//    ret += s;
//  }
//  return ret;
//}

//inline bool contains(const std::string& s1, const std::string& s2)
//{
//  return (s1.find(s2) != std::string::npos);
//}
//
//template<typename Out>
//void split(const std::string& s, char delim, Out result)
//{
//  std::stringstream ss(s);
//  std::string item;
//  while (std::getline(ss, item, delim))
//  {
//    result.push_back(item);
//  }
//}
//
//std::vector<std::string> split(const std::string& s, char delim)
//{
//  std::vector<std::string> elems;
//  split(s, delim, elems);
//  return elems;
//}

//std::string replace_all(const std::string& data,
//    const std::string& to_find, const std::string& replace_with)
//{
//  std::string ret;
//  // Get the first occurrence
//  size_t pos = data.find(to_find);
//
//  // Repeat till end is reached
//  while (pos != std::string::npos)
//  {
//    // Replace this occurrence of Sub String
//    data.replace(pos, to_find.size(), replace_with);
//    // Get the next occurrence from the current position
//    pos = data.find(to_find, pos + replace_with.size());
//  }
//
//  return ret;
//}