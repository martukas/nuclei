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

#ifndef GW_DataQuality_H
#define GW_DataQuality_H

#include <string>

class DataQuality
{
public:
  enum EnumQuality { kKnown = 0, kUnknown = 1, kTentative = 2, kTheo = 3, kAbout = 4 } ;

public:
  DataQuality();
  explicit DataQuality(DataQuality::EnumQuality info) { set_quality(info); }
  DataQuality(const DataQuality &);
  virtual ~DataQuality();

  static EnumQuality quality(const std::string);
  static std::string strip_qualifiers(const std::string original);
  std::string add_qualifiers(const std::string original, const std::string unknown = "?") const;

  virtual bool is_quality(EnumQuality) const;
  virtual void set_quality(DataQuality::EnumQuality);
  virtual void set_quality(const std::string st) { quality_ = quality(st) ; }

protected:
  EnumQuality quality_;     // set a flag to a given data
};

// inline members
inline bool DataQuality::is_quality(EnumQuality info) const { return quality_ == info ;}
inline void DataQuality::set_quality(DataQuality::EnumQuality info) { quality_ = info; }


#endif
