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
#ifndef FT_INTERFACE_SERIAL_H
#define FT_INTERFACE_SERIAL_H

#include "ftapi.h"

namespace ftapi {
  class FtInterfaceSerial: public FtInterface {
    protected:
    static unsigned const char INTELLI_IF_REFRESH[];
    static unsigned const char INTELLI_IF_QUERY_EX = 0xC6;
    static unsigned const char INTELLI_IF_QUERY_EY = 0xCA;
    static unsigned const char ROBO_IF_INIT[];
    static unsigned const char ROBO_IF_OFF = 0xA2;
    static unsigned const char ROBO_IF_COMPLETE = 0xF2;
    static unsigned const char ROBO_IF_INTERN_SERIAL[];
    static unsigned const char ROBO_IF_INTERN_FIRMWARE[];
    
    // constants for extension bits (ABF_IF_COMPLETE)
    static const unsigned char EXT_S1 = 1;
    static const unsigned char EXT_S2 = 2;
    static const unsigned char EXT_S3 = 4;
    static const unsigned char EXT_RADIO_COMM = 16;
    static const unsigned char EXT_INTERNET   = 32;
  };

} // namespace ftapi  
#endif
// EOF
