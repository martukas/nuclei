/***************************************************************************
  *   Copyright (C) 2004 by Olivier Stezowski                               *
  *   stezow(AT)ipnl.in2p3.fr                                               *
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

#ifndef GW_PARITY_H
#define GW_PARITY_H

#include <InfoData.h>
#include <iostream>

class Parity : public QualifiedData
{
public:
  enum class EnumParity { kMinus = -1, kPlus = 1 } ;

public:
  Parity() {}

  Parity(const Parity &other)
    : QualifiedData(other)
    , parity_(other.parity_)
  {}

  friend bool operator==(const Parity &left, const Parity &right);

  virtual void from_string(const std::string s);
  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

private:
  EnumParity  parity_ {EnumParity::kPlus};
};

#endif
