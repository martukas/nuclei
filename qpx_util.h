/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      TableChanSettings - tree for displaying and manipulating
 *      channel settings and chosing detectors.
 *
 ******************************************************************************/

#ifndef QPX_UTIL_H_
#define QPX_UTIL_H_

#include <string>
#include <sstream>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/algorithm/string.hpp>

const std::vector<std::string> k_UTF_superscripts = {
  "\u2070", "\u00B9", "\u00B2",
  "\u00B3", "\u2074", "\u2075",
  "\u2076", "\u2077", "\u2078",
  "\u2079"
};

const std::vector<std::string> k_UTF_subscripts = {
  "\u2080", "\u2081", "\u2082",
  "\u2083", "\u2084", "\u2085",
  "\u2086", "\u2087", "\u2088",
  "\u2089"
};

inline std::string to_str_precision(double number, int precision = -1) {
  std::ostringstream ss;
  if (precision < 0)
    ss << number;
  else
    ss << std::setprecision(precision) << number;
  return ss.str();
}

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

inline std::string to_str_decimals(double number, int decimals = 0) {
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(decimals) << number;
  return ss.str();
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
  for(int i=0;i<st.size();i++) {
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
    exponent = boost::lexical_cast<double>(expstr);

  int sigpos = 0;
  size_t pointpos = mantissa.find('.');
  if (pointpos != std::string::npos)
    sigpos -= mantissa.size() - pointpos - 1;

  // return factor for shifting according to exponent
  return pow(10.0, double(sigpos + exponent));
}

inline std::string UTF_superscript(int32_t value) {
  if (value < 0)
    return "\u207B" + UTF_superscript(-value);
  else if (value < 10)
    return k_UTF_superscripts[value];
  else
    return UTF_superscript(value / 10) + UTF_superscript(value % 10);
}

inline std::string UTF_subscript(int32_t value) {
  if (value < 0)
    return "\u208B" + UTF_subscript(-value);
  else if (value < 10)
    return k_UTF_subscripts[value];
  else
    return UTF_subscript(value / 10) + UTF_subscript(value % 10);
}

inline std::string UTF_superscript_dbl(double value, uint16_t decimals) {
  std::string sign = (value < 0) ? "\u207B" : "";
  value = std::abs(value);
  uint32_t upper = std::floor(value);
  std::string result = UTF_superscript(upper);
  if (decimals) {
    uint32_t lower = std::round((value - upper) * pow(10.0, decimals));
    if (lower)
      result += "\u00B7" + UTF_superscript(lower);
  }
  if (result != "\u2070")
    return sign + result;
  else
    return result;
}

inline std::string UTF_subscript_dbl(double value, uint16_t decimals) {
  std::string sign = (value < 0) ? "\u208B" : "";
  value = std::abs(value);
  uint32_t upper = std::floor(value);
  std::string result = UTF_subscript(upper);
  if (decimals) {
    uint32_t lower = std::round((value - upper) * pow(10.0, decimals));
    if (lower)
      result += "." + UTF_subscript(lower);
  }
  if (result != "\u2080")
    return sign + result;
  else
    return result;
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

inline std::string itobin64 (uint64_t bin)
{
  uint32_t lo = bin & 0x00000000FFFFFFFF;
  uint32_t hi = (bin >> 32) & 0x00000000FFFFFFFF;

  return (itobin32(hi) + " " + itobin32(lo));
}


inline std::string itohex64 (uint64_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint64_t)*2)
         << std::hex << bin;
  return stream.str();
}

inline std::string itohex32 (uint32_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint32_t)*2)
         << std::hex << bin;
  return stream.str();
}

inline std::string itohex16 (uint16_t bin)
{
  std::stringstream stream;
  stream << std::uppercase << std::setfill ('0') << std::setw(sizeof(uint16_t)*2)
         << std::hex << bin;
  return stream.str();
}

inline boost::posix_time::ptime from_iso_extended(std::string str)
{
  boost::posix_time::ptime tm;
  if (str.empty())
    return tm;
  boost::posix_time::time_input_facet *tif = new boost::posix_time::time_input_facet;
  tif->set_iso_extended_format();
  std::stringstream iss(str);
  iss.imbue(std::locale(std::locale::classic(), tif));
  iss >> tm;
  return tm;
}

inline boost::posix_time::ptime from_custom_format(std::string str, std::string format)
{
  boost::posix_time::ptime tm;
  if (str.empty())
    return tm;
  boost::posix_time::time_input_facet
      *tif(new boost::posix_time::time_input_facet(format));
  std::stringstream iss(str);
  iss.imbue(std::locale(std::locale::classic(), tif));
  iss >> tm;
  return tm;
}

#endif
