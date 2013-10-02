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

#include "ftinternal.h"

namespace ftapi {

  unsigned short crc16Helper(unsigned short oldcrc, unsigned char data) {
    unsigned short k = (((oldcrc >> 8) ^ data) & 0xff) << 8;
    unsigned short crc = 0;
    unsigned char bits = 8;
    do {
      if (( crc ^ k ) & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
      k <<= 1;
    }  while (--bits);
    return ((oldcrc << 8) ^ crc);
  }
  
  unsigned short crc16(size_t size, unsigned char* buffer) {
    static const unsigned short initialCrc = 0xffff;
    unsigned short crc ( initialCrc );
    while (size > 0) {
      --size;
      crc = crc16Helper(crc, *buffer);
      ++buffer;
    }
    return crc;
  }

}

