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

#ifndef GW_PARITY_H
#define GW_PARITY_H

#include <InfoData.h>
#include <iostream>

class Parity : public DataQuality
{
public:
  enum EnumParity { kMinus = -1, kPlus = 1 } ;

private:
  int16_t parity_;    // parity value

public:
  Parity();
  Parity(const Parity &p) : DataQuality(p) { parity_ = p.parity_; }
  explicit Parity(Parity::EnumParity p) { set_parity(p); }
  virtual ~Parity();

  void set_parity(Parity::EnumParity);
  bool is_parity(Parity::EnumParity) const;

  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

  virtual void from_string(const std::string s) ;

  Parity& operator*= (Parity& p);

};
// inline members
inline void Parity::set_parity(Parity::EnumParity p) { parity_ = p; }
inline bool Parity::is_parity(Parity::EnumParity p) const { return p == parity_; }

std::ostream & operator << (std::ostream &, const Parity &);

#endif
