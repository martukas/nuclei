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

#ifndef GW_SPIN_H
#define GW_SPIN_H

#include "InfoData.h"
#include <iostream>

class Spin : public QualifiedData
{
public:
  Spin() {}
  Spin(uint16_t num, uint16_t denom);
  Spin(const Spin &spin);
  static Spin from_string(const std::string& s); //from_ensdf?

  void set(uint16_t num, uint16_t denom = 1);
  uint16_t numerator()   const  { return numerator_; }
  uint16_t denominator() const  { return denominator_; }

  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;

  // some operators
  Spin& operator+=(uint16_t value);
  Spin& operator++();
  Spin operator++(int);
  Spin& operator--();
  Spin operator--(int);
  Spin operator - () const;
  Spin& operator+=(Spin sp);
  bool operator == (const Spin &s) const;
  bool operator<= (const Spin &s) const;
  bool operator< (const Spin &s) const;
  bool operator>= (const Spin &s) const;
  bool operator> (const Spin &s) const;

protected:
  uint16_t numerator_ {0};
  uint16_t denominator_ {1}; // should be 1 or 2 for a spin

  float to_float() const;
};

#endif
