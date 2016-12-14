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

void Parity::from_string(const std::string s)
{
  quality_ = quality_of(s);
  if ( boost::contains(s, "-") )
    parity_ = EnumParity::kMinus;
  else
    parity_ = EnumParity::kPlus;
}

const std::string Parity::to_string() const
{
  if ( quality_ == DataQuality::kUnknown )
    return "";
  else if ( parity_ == EnumParity::kPlus )
    return "+";
  else
    return "-";
}

const std::string Parity::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}


bool operator==(const Parity &left, const Parity &right)
{
  return (left.parity_ == right.parity_);
}
