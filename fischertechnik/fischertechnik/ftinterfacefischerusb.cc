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
#ifdef HAVE_FISCHERUSB
#include "ftinterfacefischerusb.h"
#include <fischerusb.h>
#include <fcntl.h>   /* File control definitions */
#include <sys/ioctl.h>
#include <iostream>
#include <errno.h>   /* EAGAIN, etc. */
#include <cassert>
#include <strings.h>
#include <string.h>


/** @file
implementation */
namespace ftapi {


FtInterfaceFischerUsb::FtInterfaceFischerUsb(std::string device) throw(XFtComm)
 {
  fd = open(device.c_str(), O_RDWR);
  if (fd == -1) {
   /*
    * Could not open the port.
    */
    throw XFtComm("FtInterfaceFischerUsb: unable to open device");
  } else {
    // ok, port opened
    _info.device = device;
    _info.serialNo = ioctl(fd, ROBO_IF_IOCQSERIAL);
    int bcdRevision = ioctl(fd, ROBO_IF_IOCQREVISION);
    _info.firmwareVer[0] = (bcdRevision >> 24 ) & 0xff;
    _info.firmwareVer[1] = (bcdRevision >> 16 ) & 0xff;
    _info.firmwareVer[2] = (bcdRevision >> 8 ) & 0xff;
    _info.firmwareVer[3] = bcdRevision & 0xff;
     unsigned char buffer[ROBO_IF_IOC_SIZE];
    bzero(&buffer, sizeof(buffer));
    buffer[ 0]=ROBO_IF_COMPLETE;

    // now set/get one time (to see if it works)
    int res=0;
    do {   
      res=ioctl(fd, ROBO_IF_IOCXQSET, &buffer);
      if (res!=0) {
        if ((res==-1) && (EAGAIN==errno))  {
        } else if (res!=-EAGAIN) {
          throw XFtComm("FtInterfaceFischerUsb: ioctl failed");
        } else {
        }
      }
    } while (res!=0);

    // installed modules
    _info.ifType = FT_IF_ROBO;
    _info.ifConn = FT_IC_USB;
/* we are unable to detect extensions: 
   the following code does not work properly
    const unsigned char extensions = buffer[15+17];
    _info.slavePresent[0] = ((extensions & EXT_S1)!=0);
    _info.slavePresent[1] = ((extensions & EXT_S2)!=0);
    _info.slavePresent[2] = ((extensions & EXT_S3)!=0);
    _info.radioCommModulePresent = ((extensions & EXT_RADIO_COMM)!=0);
    _info.internetModulePresent  = ((extensions & EXT_INTERNET)!=0);
*/
  }
}

FtInterfaceFischerUsb::~FtInterfaceFischerUsb() {
  assert(-1 != fd); // dtor should not be reached if if cannot be opened
  close(fd);                      // close port, do not care if it works
}


void FtInterfaceFischerUsb::getIfInfo(IfInfo &info) {
  info = _info;
}


void FtInterfaceFischerUsb::writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm) {
  unsigned char buffer[ROBO_IF_IOC_SIZE];
  assert(ROBO_IF_IOC_SIZE >= ROBO_IF_COMPLETE_WRITE_SIZE + ROBO_IF_COMPLETE_READ_SIZE);
  memset(&buffer, 0, sizeof(buffer));
  int res=0;
  switch(_info.ifType) {
    case FT_IF_ROBO:
    case FT_IF_RF_DATA:
      roboIfCopyToBuffer(outputs, buffer);
      do {
        res=ioctl(fd, ROBO_IF_IOCXQSET, &buffer);
        if (res!=0) {
          if ((res==-1) && (EAGAIN==errno))  {
    //         std::cerr<<strerror(errno)<<std::endl;
          } else if (res!=-EAGAIN) {
    //         std::cerr<<res<<std::endl;
            throw XFtComm("FtInterfaceFischerUsb: ioctl failed");
          } else {
    //         std::cerr<<"again"<<std::endl;
          }
        }
      } while (res!=0);
      inputs = roboIfCopyFromBuffer(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE]));
      break;

    case FT_IF_IOE:
      roboIoeCopyToBuffer(outputs, buffer);
      do {
        res=ioctl(fd, ROBO_IF_IOCXQSET, &buffer);
        if (res!=0) {
          if ((res==-1) && (EAGAIN==errno))  {
    //         std::cerr<<strerror(errno)<<std::endl;
          } else if (res!=-EAGAIN) {
    //         std::cerr<<res<<std::endl;
            throw XFtComm("FtInterfaceFischerUsb: ioctl failed");
          } else {
    //         std::cerr<<"again"<<std::endl;
          }
        }
      } while (res!=0);
      inputs = roboIoeCopyFromBuffer(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE]));
      break;

    default:
      assert(0);
  }
}

} // namespace
#endif
// EOF
