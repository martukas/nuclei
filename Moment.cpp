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

#include "Moment.h"
#include <string>
#include <boost/algorithm/string.hpp>
#include "qpx_util.h"

Moment::Moment()
{
}


Moment::~Moment()
{
}

bool Moment::valid() const
{
  return moment_.hasFiniteValue();
}

Moment Moment::from_ensdf(std::string record)
{
  std::string value_str;
  std::string uncert_str;
  std::string references_str;

  std::vector<std::string> momentparts;
  std::string rec_copy = trim_all(record);
  boost::trim(rec_copy);
  boost::split(momentparts, rec_copy, boost::is_any_of(" \r\t\n\0"));
  if (momentparts.size() >= 1)
    value_str = momentparts[0];
  if (momentparts.size() >= 2)
    uncert_str = momentparts[1];
  if (momentparts.size() >= 3)
    references_str = momentparts[2];

  if (UncertainDouble::is_uncert(value_str))
  {
    value_str = uncert_str;
    uncert_str = momentparts[0];
  }

  Moment ret;
  ret.moment_ = UncertainDouble::from_nsdf(value_str, uncert_str);

  if (!references_str.empty()) {
    boost::replace_all(references_str, "(", "");
    boost::replace_all(references_str, ")", "");
    boost::split(ret.references_, references_str, boost::is_any_of(","));
  }

  return ret;
}


const std::string Moment::to_string() const
{
  return moment_.to_string(false);
}

const std::string Moment::to_markup() const
{
  return moment_.to_markup();
}


//const std::string Moment::to_qualified_string(const std::string unknown) const
//{
//  std::string ret = to_string();
//  if (!references_.empty()) {
//    ret += " Ref[ ";
//    for (auto &q : references_)
//      ret += q + " ";
//    ret += "]";
//  }
//  return ret;
//}


std::ostream & operator << (std::ostream &out, const Moment &p)
{
  out << p.to_string();
  return out;
}
