#include "InfoData.h"
#include <boost/algorithm/string.hpp>

DataQuality quality_of(const std::string& s)
{
  std::string st = s;
  int16_t nb, nb_unkown, nb_theo_left, nb_theo_right, nb_tenta_left, nb_tenta_right, nb_about;
  nb = nb_unkown = nb_theo_left = nb_theo_right = nb_tenta_left = nb_tenta_right = nb_about = 0;

  // reads the string and counts the specfics characters
  size_t end = st.length();
  for(size_t i = 0 ; i < end; i++)
  {
    if ( st[i] == '?' ) { nb_unkown++; nb++; }
    if ( st[i] == '(' ) { nb_tenta_left++; nb++;}  ; if ( st[i] == ')' ) { nb_tenta_right++; nb++;}
    if ( st[i] == '[' ) { nb_theo_left++; nb++; }  ; if ( st[i] == ']' ) { nb_theo_right++; nb++; }
    if ( st[i] == '~' ) { nb_about++; nb++; }
  }
  // set the informations
  if ( nb  > 2 ) return DataQuality::kUnknown;
  if ( nb == 0 ) return DataQuality::kKnown;

  if ( nb == 1 ) {
    if ( nb_about  == 1 ) return DataQuality::kAbout;
    else return DataQuality::kUnknown;
  }
  else { // nb = 2
    if ( nb_tenta_left == 1 && nb_tenta_right == 1 ) return DataQuality::kTentative;
    if (  nb_theo_left == 1 &&  nb_theo_right == 1  ) return DataQuality::kTheoretical;
  }
  return DataQuality::kUnknown;
}

std::string strip_qualifiers(const std::string& original)
{
  std::string ret = original;
  switch( quality_of(original) )
  {
  case DataQuality::kAbout:
    boost::replace_all(ret, "~"," ");
    break;
  case DataQuality::kTentative:
    boost::replace_all(ret, "("," ");
    boost::replace_all(ret, "("," ");
    break;
  case DataQuality::kTheoretical:
    boost::replace_all(ret, "["," ");
    boost::replace_all(ret, "["," ");
    break;
  default:
    break;
  }
  boost::trim(ret);
  return ret;
}

std::string add_qualifiers(const std::string& original,
                           const DataQuality& quality,
                           const std::string& unknown)
{
  if ( quality == DataQuality::kUnknown )
    return unknown;

  if ( quality == DataQuality::kTentative )
    return "(" + original + ")";
  if ( quality == DataQuality::kTheoretical )
    return "[" + original + "]";
  if ( quality == DataQuality::kAbout )
    return "~" + original;

  return original;
}

std::string QualifiedData::add_qualifiers(const std::string& original,
                                          const std::string& unknown) const
{
  return ::add_qualifiers(original, quality_, unknown);
}

