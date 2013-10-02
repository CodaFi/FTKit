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
 *  I f I n p u t s                                                        *
***************************************************************************/

IfInputs::IfInputs(): ay(0), az(0), a1(0), a2(0), supply(0), as1Digital(false), 
 as2Digital(false), dist1(0), dist2(0), timeInMs(0), irKey(0), code2(false) {
  data[0] = 0; 
  data[1] = 0; 
  data[2] = 0; 
  data[3] = 0; 
  ax[0] = 0; 
  ax[1] = 0; 
  ax[2] = 0; 
  ax[3] = 0; 
}

bool IfInputs::pin(ft_module_type module, ft_pin pin) { 
  assert( (FT_MASTER==module) || (FT_SLAVE1==module) ||
          (FT_SLAVE2==module) || (FT_SLAVE3==module) );
  // assert for ftpin would be nice but inefficient
  return ( (data[module] & pin ) != 0 ); 
}

} // namespace ftapi
// EOF
