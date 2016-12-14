/***************************************************************************
  *   Copyright (C) 2004 by Olivier Stezowski                               *
  *   stezow(AT)ipnl.in2p3.fr                                                  *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/

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

