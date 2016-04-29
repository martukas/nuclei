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

DataQuality::DataQuality()
  : quality_(kKnown)
{}

DataQuality::DataQuality(const DataQuality &info)
{
  quality_ = info.quality_;
}

DataQuality::~DataQuality()
{
}

DataQuality::EnumQuality DataQuality::quality(const std::string s)
{
  std::string st = s;
  int16_t nb, nb_unkown, nb_theo_left, nb_theo_right, nb_tenta_left, nb_tenta_right, nb_about;
  nb = nb_unkown = nb_theo_left = nb_theo_right = nb_tenta_left = nb_tenta_right = nb_about = 0;

  // reads the string and counts the specfics characters
  size_t end = st.length();
  for( size_t i = 0 ; i < end; i++ ){
    if ( st[i] == '?' ) { nb_unkown++; nb++; }
    if ( st[i] == '(' ) { nb_tenta_left++; nb++;}  ; if ( st[i] == ')' ) { nb_tenta_right++; nb++;}
    if ( st[i] == '[' ) { nb_theo_left++; nb++; }  ; if ( st[i] == ']' ) { nb_theo_right++; nb++; }
    if ( st[i] == '~' ) { nb_about++; nb++; }
  }
  // set the informations
  if ( nb  > 2 ) return kUnknown;
  if ( nb == 0 ) return kKnown;

  if ( nb == 1 ) {
    if ( nb_about  == 1 ) return kAbout;
    else return kUnknown;
  }
  else { // nb = 2
    if ( nb_tenta_left == 1 && nb_tenta_right == 1 ) return kTentative;
    if (  nb_theo_left == 1 &&  nb_theo_right == 1  ) return kTheo;
  }
  return kUnknown;
}

std::string DataQuality::strip_qualifiers(const std::string original)
{
  std::string ret = original;
  switch( DataQuality::quality(original) ){
  case kAbout:
    boost::replace_all(ret, "~"," ");
    break;
  case kTentative:
    boost::replace_all(ret, "("," ");
    boost::replace_all(ret, "("," ");
    break;
  case kTheo:
    boost::replace_all(ret, "["," ");
    boost::replace_all(ret, "["," ");
    break;
  default:
    break;
  }
  boost::trim(ret);
  return ret;
}

std::string DataQuality::add_qualifiers(const std::string original, const std::string unknown) const
{
  if ( is_quality(DataQuality::kUnknown) )
    return unknown;

  if ( is_quality(DataQuality::kTentative) )
    return "(" + original + ")";
  if ( is_quality(DataQuality::kTheo) )
    return "[" + original + "]";
  if ( is_quality(DataQuality::kAbout) )
    return "~" + original;

  return original;
}

