#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <boost/algorithm/string.hpp>

inline std::string trim_all(std::string text)
{
  std::istringstream iss(text);
  text = "";
  std::string s;
  while(iss >> s){
    if ( text != "" ) text += " " + s;
    else text = s;
  }
  return text;
}

inline std::string join(const std::vector<std::string> &list, std::string spacer = "")
{
  std::string ret;
  for (size_t i=0; i < list.size(); ++i) {
    ret += list.at(i);
    if ((i > 0) && ((i+1) < list.size()))
      ret += spacer;
  }
  return ret;
}

template<typename T> inline bool is_number(T x)
{
  std::string s;
  std::stringstream ss;
  ss << x;
  ss >>s;
  if(s.empty() || std::isspace(s[0]) || std::isalpha(s[0])) return false ;
  char * p ;
  strtod(s.c_str(), &p) ;
  return (*p == 0) ;
}

inline uint16_t sig_digits(std::string st)
{
  boost::to_lower(st);
  boost::replace_all(st, "+", "");
  boost::replace_all(st, "-", "");
  if (boost::contains(st, "e")) {
    size_t l = st.find('e');
    st = st.substr(0,l);
  }
  //assume only one number in string
  uint16_t count=0; bool past_zeros = false;
  for(size_t i=0;i<st.size();i++) {
    bool digit = std::isdigit(st[i]);
    if (digit && (st[i] != '0'))
      past_zeros = true;
    if(past_zeros && digit)
      count++;
  }
  return count;
}

inline int16_t order_of(double val)
{
  return  std::floor(std::log10(std::abs(val)));
}

inline double get_precision(std::string value)
{
  boost::trim(value);
  boost::trim_if(value, boost::is_any_of("+-"));
  std::vector<std::string> parts;
  boost::split(parts, value, boost::is_any_of("Ee"));

  std::string mantissa;
  std::string expstr;

  if (parts.size() >= 1)
    mantissa = parts.at(0);
  if (parts.size() >= 2)
    expstr = parts.at(1);

  int exponent = 0;
  if (!expstr.empty() && is_number(expstr))
    exponent = std::stod(expstr);

  int sigpos = 0;
  size_t pointpos = mantissa.find('.');
  if (pointpos != std::string::npos)
    sigpos -= mantissa.size() - pointpos - 1;

  // return factor for shifting according to exponent
  return pow(10.0, double(sigpos + exponent));
}

inline std::string itobin16 (uint16_t bin)
{
  std::stringstream ss;
  for (int k = 0; k < 16; ++k) {
    if (bin & 0x8000)
      ss << "1";
    else
      ss << "0";
    bin <<= 1;
  }
  return ss.str();
}

inline std::string itobin32 (uint32_t bin)
{
  uint16_t lo = bin & 0x0000FFFF;
  uint16_t hi = (bin >> 16) & 0x0000FFFF;

  return (itobin16(hi) + " " + itobin16(lo));
}

