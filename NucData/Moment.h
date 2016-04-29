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

#ifndef GW_MOMENT_H
#define GW_MOMENT_H

#include <iostream>
#include "UncertainDouble.h"
#include <vector>
#include <string>

class Moment
{

private:
  UncertainDouble moment_;    // moment value
  std::vector<std::string> references_;

public:
  Moment();
  Moment(const Moment &p) { moment_ = p.moment_; }
  virtual ~Moment();

  bool valid() const;

  const std::string to_string() const;
  const std::string to_markup() const;

  static Moment from_ensdf(std::string s);
};

std::ostream & operator << (std::ostream &, const Moment &);

#endif
