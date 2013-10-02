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

IfInfo::IfInfo(): ifType(FT_IF_UNKNOWN), ifConn(FT_IC_UNKNOWN), serialNo(0),
 rfdSerialNo(0)
// radioCommModulePresent(false),
// internetModulePresent(false) 
 { 
 firmwareVer[0] = 0;
 firmwareVer[1] = 0;
 firmwareVer[2] = 0;
 firmwareVer[3] = 0;
 rfdFirmwareVer[0] = 0;
 rfdFirmwareVer[1] = 0;
 rfdFirmwareVer[2] = 0;
 rfdFirmwareVer[3] = 0;
 /* we are unable to detect extensions: 
   the following code does not work properly
 slavePresent[0] = 0;
 slavePresent[1] = 0;
 slavePresent[2] = 0;*/
}

} // namespace ftapi
// EOF

