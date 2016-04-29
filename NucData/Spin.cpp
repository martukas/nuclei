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

#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "Spin.h"
#include "custom_logger.h"

Spin::Spin()
  : DataQuality()
{
  numerator_ = 0; denominator_ = 1;
}

Spin::Spin(uint16_t num, uint16_t denom)
  : DataQuality()
{
  set(num, denom);
}

Spin::~Spin()
{
}

uint16_t Spin::doubled_spin() const
{
  if ( denominator_ == 1 )
    return 2*numerator_;
  return numerator_;
}

Spin& Spin::operator++()
{
  numerator_ += denominator_;
  return *this;
}

Spin Spin::operator++(int)
{
  Spin result(*this);
  Spin::operator++();
  return result;
}

Spin& Spin::operator--()
{
  numerator_ -= denominator_;
  return *this;
}

Spin Spin::operator--(int)
{
  Spin result(*this);
  Spin::operator--();
  return result;
}

Spin Spin::operator - () const
{
  Spin result(*this);
  result.set(-numerator_,denominator_);
  return result;
}

bool Spin::operator == (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return false;
}

bool Spin::operator<= (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return to_float() <= s.to_float();
}
bool Spin::operator< (const Spin &s) const
{
  return to_float() < s.to_float();
}

bool Spin::operator>= (const Spin &s) const
{
  if ( numerator_ == s.numerator_ && denominator_ == s.denominator_ )
    return true;
  return to_float() >= s.to_float();
}

bool Spin::operator> (const Spin &s) const
{
  return to_float() > s.to_float();
}

void Spin::from_string(const std::string s)
{
  std::string st = DataQuality::strip_qualifiers(s);
  std::istringstream input; input.clear();
  if ( boost::contains(st, "/") )
  {
    // not an integer
    boost::replace_all(st, "/", " ");
    input.str(st);
    input >> numerator_ >> denominator_;
  }
  else
  {
    input.str(st);
    input >> numerator_;
    denominator_ = 1;
  }
  if ( input.fail() )
    set_quality(kUnknown);
  else
    set_quality( DataQuality::quality(s) );
}

void Spin::set(uint16_t num, uint16_t denom)
{
  numerator_ = num; denominator_ = denom;
  if ( denominator_ < 1 || denominator_ > 2 ) {
    WARN << "Spin should be an integer or half an integer!     "
         << "[" << numerator_ << "/" << denominator_ << "]"
         << " --> "
         << "[" << numerator_ << "/1]";
    denominator_ = 1;
  }
}

Spin& Spin::operator+=(uint16_t value)
{
  numerator_ += denominator_*value;
  return *this;
}

Spin& Spin::operator+=(Spin sp)
{
  if (denominator_ == sp.denominator_) {
    numerator_ += sp.numerator_;
  } else {
    numerator_ = denominator_*sp.numerator_ + numerator_*sp.denominator_;
    denominator_ = std::max(denominator_, sp.denominator_);
  }
  return *this;
}

const std::string Spin::to_string() const
{
  if ( is_quality(DataQuality::kUnknown) )
    return "";
  else if ( denominator_ == 1 )
    return std::to_string(numerator_);
  else
    return std::to_string(numerator_) + "/" + std::to_string(denominator_);
}

const std::string Spin::to_qualified_string(const std::string unknown) const
{
  return add_qualifiers(to_string(), unknown);
}

std::ostream & operator << (std::ostream &out, const Spin &spin)
{
  out << spin.to_qualified_string("?");
  return out;
}
