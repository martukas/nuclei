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

#include "Parity.h"
#include <string>
#include <boost/algorithm/string.hpp>

Parity::Parity()
  : DataQuality()
{
  parity_ = kPlus;
}


Parity::~Parity()
{
}

void Parity::from_string(const std::string s)
{
  if ( boost::contains(s, "-") )
    parity_ = kMinus;
  else
    parity_ = kPlus;

  set_quality( DataQuality::quality(s) );
}

Parity& Parity::operator*= (Parity& p)
{
  parity_ *= p.parity_;
  return *this;
}

const std::string Parity::to_string() const
{
  if ( is_quality(DataQuality::kUnknown) )
    return "";
  else if ( is_parity(Parity::kPlus) )
    return "+";
  else
    return "-";
}

const std::string Parity::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}


std::ostream & operator << (std::ostream &out, const Parity &p)
{
  out << p.to_qualified_string("?");
  return out;
}
