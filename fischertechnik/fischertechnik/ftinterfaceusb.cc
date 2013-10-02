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
#include "ftinterfaceusb.h"

/** @file
implementation */
namespace ftapi {


void FtInterfaceUsb::roboIfCopyToBuffer(const IfOutputs &outputs, unsigned char *buffer) {
  if (NULL == buffer) throw XFtComm("buffer != NULL");
  buffer[ 0]=ROBO_IF_COMPLETE;
  buffer[ 1]=outputs.data[FT_MASTER];
  buffer[ 2]=(((outputs.speed[FT_MASTER][0])&7)   )
            +(((outputs.speed[FT_MASTER][1])&7)<<3)
            +(((outputs.speed[FT_MASTER][2])&3)<<6);
  buffer[ 3]=(((outputs.speed[FT_MASTER][2])&4)>>2)
            +(((outputs.speed[FT_MASTER][3])&7)<<1)
            +(((outputs.speed[FT_MASTER][4])&7)<<4)
            +(((outputs.speed[FT_MASTER][5])&1)<<7);
  buffer[ 4]=(((outputs.speed[FT_MASTER][5])&6)>>1)
            +(((outputs.speed[FT_MASTER][6])&7)<<2)
            +(((outputs.speed[FT_MASTER][7])&7)<<5);
  buffer[ 5]=outputs.data[FT_SLAVE1];
  buffer[ 6]=(((outputs.speed[FT_SLAVE1][0])&7)   )
            +(((outputs.speed[FT_SLAVE1][1])&7)<<3)
            +(((outputs.speed[FT_SLAVE1][2])&3)<<6);
  buffer[ 7]=(((outputs.speed[FT_SLAVE1][2])&4)>>2)
            +(((outputs.speed[FT_SLAVE1][3])&7)<<1)
            +(((outputs.speed[FT_SLAVE1][4])&7)<<4)
            +(((outputs.speed[FT_SLAVE1][5])&1)<<7);
  buffer[ 8]=(((outputs.speed[FT_SLAVE1][5])&6)>>1)
            +(((outputs.speed[FT_SLAVE1][6])&7)<<2)
            +(((outputs.speed[FT_SLAVE1][7])&7)<<5);
  buffer[ 9]=outputs.data[FT_SLAVE2];
  buffer[10]=(((outputs.speed[FT_SLAVE2][0])&7)   )
            +(((outputs.speed[FT_SLAVE2][1])&7)<<3)
            +(((outputs.speed[FT_SLAVE2][2])&3)<<6);
  buffer[11]=(((outputs.speed[FT_SLAVE2][2])&4)>>2)
            +(((outputs.speed[FT_SLAVE2][3])&7)<<1)
            +(((outputs.speed[FT_SLAVE2][4])&7)<<4)
            +(((outputs.speed[FT_SLAVE2][5])&1)<<7);
  buffer[12]=(((outputs.speed[FT_SLAVE2][5])&6)>>1)
            +(((outputs.speed[FT_SLAVE2][6])&7)<<2)
            +(((outputs.speed[FT_SLAVE2][7])&7)<<5);
  buffer[13]=outputs.data[FT_SLAVE3];
  buffer[14]=(((outputs.speed[FT_SLAVE3][0])&7)   )
            +(((outputs.speed[FT_SLAVE3][1])&7)<<3)
            +(((outputs.speed[FT_SLAVE3][2])&3)<<6);
  buffer[15]=(((outputs.speed[FT_SLAVE3][2])&4)>>2)
            +(((outputs.speed[FT_SLAVE3][3])&7)<<1)
            +(((outputs.speed[FT_SLAVE3][4])&7)<<4)
            +(((outputs.speed[FT_SLAVE3][5])&1)<<7);
  buffer[16]=(((outputs.speed[FT_SLAVE3][5])&6)>>1)
            +(((outputs.speed[FT_SLAVE3][6])&7)<<2)
            +(((outputs.speed[FT_SLAVE3][7])&7)<<5);
}


IfInputs FtInterfaceUsb::roboIfCopyFromBuffer(unsigned char *buffer) {
  if (NULL == buffer) throw XFtComm("buffer != NULL");
  IfInputs inputs;
  inputs.data[FT_MASTER] = buffer[ 0];
  inputs.data[FT_SLAVE1] = buffer[ 1];
  inputs.data[FT_SLAVE2] = buffer[ 2];
  inputs.data[FT_SLAVE3] = buffer[ 3];
  inputs.ax[FT_MASTER]   = buffer[ 4] + (static_cast<int>((buffer[ 8]   )&3)<<8);
  inputs.ay              = buffer[ 5] + (static_cast<int>((buffer[ 8]>>2)&3)<<8);
  inputs.a1              = buffer[ 6] + (static_cast<int>((buffer[ 8]>>4)&3)<<8);
  inputs.a2              = buffer[ 7] + (static_cast<int>((buffer[ 8]>>6)&3)<<8);
  inputs.az              = buffer[ 9] + (static_cast<int>((buffer[13]   )&3)<<8);
  inputs.dist1           = buffer[10] + (static_cast<int>((buffer[13]>>2)&3)<<8);
  inputs.dist2           = buffer[11] + (static_cast<int>((buffer[13]>>4)&3)<<8);
  inputs.supply          = buffer[12] + (static_cast<int>((buffer[13]>>6)&3)<<8);
  // IR code, ...
  inputs.irKey           = buffer[14] & 15;
  inputs.code2           = (buffer[14] >> 4) & 1;
  inputs.as1Digital      = (buffer[14] >> 6) & 1;
  inputs.as2Digital      = (buffer[14] >> 7) & 1;
  // installed modules     buffer[15] (not to implement here)
  inputs.timeInMs        = buffer[16];
  inputs.ax[FT_SLAVE1]   = buffer[17] + (static_cast<int>((buffer[20]   )&3)<<8);
  inputs.ax[FT_SLAVE2]   = buffer[18] + (static_cast<int>((buffer[20]>>2)&3)<<8);
  inputs.ax[FT_SLAVE3]   = buffer[19] + (static_cast<int>((buffer[20]>>4)&3)<<8);

/* we are unable to detect extensions: 
   the following code does not work properly
  // update entensions
  const unsigned char extensions = buffer[15];
  _info.slavePresent[0] = ((extensions & EXT_S1)!=0);
  _info.slavePresent[1] = ((extensions & EXT_S2)!=0);
  _info.slavePresent[2] = ((extensions & EXT_S3)!=0);
  _info.radioCommModulePresent = ((extensions & EXT_RADIO_COMM)!=0);
  _info.internetModulePresent  = ((extensions & EXT_INTERNET)!=0); */
  return inputs;
}




void FtInterfaceUsb::roboIoeCopyToBuffer(const IfOutputs &outputs, unsigned char *buffer) {
  if (NULL == buffer) throw XFtComm("buffer != NULL");
  buffer[ 0]=ROBO_IF_COMPLETE;
  buffer[ 1]=outputs.data[FT_MASTER];
  buffer[ 2]=(((outputs.speed[FT_MASTER][0])&7)   )
            +(((outputs.speed[FT_MASTER][1])&7)<<3)
            +(((outputs.speed[FT_MASTER][2])&3)<<6);
  buffer[ 3]=(((outputs.speed[FT_MASTER][2])&4)>>2)
            +(((outputs.speed[FT_MASTER][3])&7)<<1)
            +(((outputs.speed[FT_MASTER][4])&7)<<4)
            +(((outputs.speed[FT_MASTER][5])&1)<<7);
  buffer[ 4]=(((outputs.speed[FT_MASTER][5])&6)>>1)
            +(((outputs.speed[FT_MASTER][6])&7)<<2)
            +(((outputs.speed[FT_MASTER][7])&7)<<5);
  buffer[ 5]=ROBO_IOE_MARKING;
}


IfInputs FtInterfaceUsb::roboIoeCopyFromBuffer(unsigned char *buffer) {
  if (NULL == buffer) throw XFtComm("buffer != NULL");
  IfInputs inputs;
  inputs.data[FT_MASTER] = buffer[ 0];
  inputs.ax[FT_MASTER]   = buffer[ 1] + (static_cast<int>((buffer[ 4]   )&3)<<8);
  inputs.a1              = buffer[ 2] + (static_cast<int>((buffer[ 4]>>2)&3)<<8);
  inputs.supply          = buffer[ 3] + (static_cast<int>((buffer[ 4]>>4)&3)<<8);
  // buffer[5] is currently unused
  return inputs;
}



} // namespace
// EOF
