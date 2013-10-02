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
#include <cassert>

namespace ftapi {

/***************************************************************************
 *  I f O u t p u t s                                                      *
***************************************************************************/

IfOutputs::IfOutputs() {
  data[0] = 0; 
  data[1] = 0; 
  data[2] = 0; 
  data[3] = 0; 
  for (int i=0; i<4; ++i) {
    for (int j=0; j<8; ++j) {
      speed[i][j] = 0;
    }
  }
}

void IfOutputs::setMotorSpeed(const ft_module_type module, const ft_motor no, const FtMotorSpeed &mSpeed) {
  assert(no>=0);
  assert(no<4);
  assert(mSpeed.get()<=7);
  assert(mSpeed.get()>=-7);
  if (mSpeed.get()>=0) {
    speed[module][no*2] = 0;
    speed[module][no*2+1] = mSpeed.get();
  } else {
    speed[module][no*2] = -mSpeed.get();
    speed[module][no*2+1] = 0;
  }
  data[module] |= 1<<(no*2);
  data[module] |= 1<<(no*2+1);
}

FtMotorSpeed IfOutputs::getMotorSpeed(const ft_module_type module, const ft_motor no) const {
  assert(no>=0);
  assert(no<4);

  // check, if port is active
  if ( (data[module] & (1 << (no * 2 + 0) ) ) == 0 ) return 0;
  if ( (data[module] & (1 << (no * 2 + 1) ) ) == 0 ) return 0;
  return (speed[module][no*2+1]-speed[module][no*2+0]);
}

} // namespace ftapi
// EOF
