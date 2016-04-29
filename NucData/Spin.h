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

class Spin : public DataQuality
{
public:
  //friend bool operator> (Spin &S1, Spin &S2);
  //friend bool operator<= (Spin &S1, Spin &S2);

  //friend bool operator< (Spin &S1, Spin &S2);
  //friend bool operator>= (Spin &S1, Spin &S2);

protected:
  uint16_t numerator_; // numerator
  uint16_t denominator_; // denominator - should be 1 or 2 for a spin

public:
  Spin();
  Spin(uint16_t num, uint16_t denom);
  Spin(const Spin &spin): DataQuality(spin) { numerator_ = spin.numerator_; denominator_ = spin.denominator_; }
  virtual ~Spin();

  void set(uint16_t num, uint16_t denom = 1);
  uint16_t spin_numerator()   const  { return numerator_; }
  uint16_t spin_denominator() const  { return denominator_; }
  float to_float() const;

  bool     is_half_int() const { return denominator_ == 2; }
  uint16_t doubled_spin() const;

  const std::string to_string() const;
  const std::string to_qualified_string(const std::string unknown = "?") const;
  virtual void from_string(const std::string s) ;

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
};

// inline members
inline float Spin::to_float() const { return float(numerator_) / float(denominator_); }
//    friend bool operator> (Spin &S1, Spin &S2);
//    inline bool operator<= (Spin &S1, Spin &S2){}
//    friend bool operator< (Spin &S1, Spin &S2);
//    friend bool operator>= (Spin &S1, Spin &S2);


std::ostream & operator << (std::ostream &, const Spin &);

#endif
