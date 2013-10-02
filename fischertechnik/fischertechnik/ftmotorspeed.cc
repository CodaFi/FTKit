/***************************************************************************
 *  ftApi - fischertechnik c++ progamming api                              *
 *  Copyright (C) 2004,2005 by                                             *
 *  Holger Friedrich <holgerf@vsi.cs.uni-frankfurt.de>,                    *
 *  Visual Sensorics and Information processing lab,                       * 
 *  University of Frankfurt                                                *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; if not, write to the Free Software    *
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA02110-1301 USA*
 ***************************************************************************/

#include "ftapi.h"

namespace ftapi {

/***************************************************************************
 *  I f I n f o                                                            *
***************************************************************************/


FtMotorSpeed::FtMotorSpeed(int speed) {
  _speed = 0;
  set(speed);
}

void FtMotorSpeed::stop() {
  _speed = 0;
}

void FtMotorSpeed::setMax() {
  _speed = _maxSpeed;
}

void FtMotorSpeed::setMin() {
  _speed = _minSpeed;
}

void FtMotorSpeed::set(const int speed) {
  if (speed > _maxSpeed) {
    setMax();
  }
  else if (speed < _minSpeed) {
    setMin();
  }
  else {
    _speed = speed;
  }
}

int FtMotorSpeed::get() const {
  return _speed;
}

const FtMotorSpeed FtMotorSpeed::operator++(const int) {
  const FtMotorSpeed oldValue = *this;
  ++(*this);
  return oldValue;
}

const FtMotorSpeed FtMotorSpeed::operator--(const int) {
  const FtMotorSpeed oldValue = *this;
  --(*this);
  return oldValue;
}

FtMotorSpeed& FtMotorSpeed::operator++() {
  *this += 1;
  return *this;
}

FtMotorSpeed& FtMotorSpeed::operator--() {
  *this -= 1;
  return *this;
}

void FtMotorSpeed::operator+=(const int speed) {
  set(_speed + speed);
}

void FtMotorSpeed::operator-=(const int speed) {
  set(_speed - speed);
}

FtMotorSpeed FtMotorSpeed::operator+(const int speed) {
  return FtMotorSpeed(get()+speed);
}

FtMotorSpeed FtMotorSpeed::operator-(const int speed) {
  return FtMotorSpeed(get()-speed);
}

FtMotorSpeed& FtMotorSpeed::operator=(const int speed) {
  set(speed);
  return *this;
}

FtMotorSpeed& FtMotorSpeed::operator=(const FtMotorSpeed &mSpeed) {
  if ( *this == mSpeed ) return *this; // self assignment
  set(mSpeed.get());
  return *this;
}

bool FtMotorSpeed::operator==(const FtMotorSpeed mspeed) {
  return ( get() == mspeed.get() );
}

bool FtMotorSpeed::operator!=(const FtMotorSpeed mspeed) {
  return ( get() != mspeed.get() );
}

bool FtMotorSpeed::operator<(const FtMotorSpeed mspeed) {
  return ( get() < mspeed.get() );
}

bool FtMotorSpeed::operator>(const FtMotorSpeed mspeed) {
  return ( get() > mspeed.get() );
}

bool FtMotorSpeed::operator<=(const FtMotorSpeed mspeed) {
  return ( get() <= mspeed.get() );
}

bool FtMotorSpeed::operator>=(const FtMotorSpeed mspeed) {
  return ( get() >= mspeed.get() );
}

} // namespace ftapi
// EOF

