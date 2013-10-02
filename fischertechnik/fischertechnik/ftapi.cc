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
#include "ftinterfaceposixserial.h"
#include "ftinterfacelibusb.h"
#include "ftinterfacefischerusb.h"
#include "ftinterfaceusbappleiokit.h"
#include <ostream>

/** @file
implementation */
#ifndef DOXYGEN_SHOULD_SKIP_THIS

namespace ftapi {


/***************************************************************************
 *  f t G e t I f F r o m F a c t o r y                                    *
***************************************************************************/

FtInterface *ftGetIfFromFactory(std::string device, ft_if_conn connection) throw (XFt) {
  switch (connection) {
#ifdef HAVE_POSIXSERIAL
    case FT_IC_SERIAL: 
      return new FtInterfacePosixSerial(device);
      break;
#endif
#ifdef HAVE_FISCHERUSB
    case FT_IC_USB: 
      return new FtInterfaceFischerUsb(device);
      break;
#endif
#ifdef HAVE_LIBUSB
    case FT_IC_LIB_USB: 
      return new FtInterfaceLibUsb(device);
      break;
#endif
#ifdef HAVE_APPLEIOKIT
    case FT_IC_USB_IOKIT:
      return new FtInterfaceUsbAppleIOKit(device);
      break;
#endif
    default: 
      throw XFt("connection type not supported");
  }
  // compiler warnings
  std::string dev(device);
}



/***************************************************************************
 *  X f t . . .                                                            *
***************************************************************************/

/// base exception class
XFt::XFt(): msg("XFt") {}
XFt::XFt(std::string message): msg("XFt: ") { msg += message; }
XFt::~XFt() throw() {}
const char* XFt::what() const throw() { return msg.c_str(); }

/// communication exception
XFtComm::XFtComm() { msg="XFtComm"; }
XFtComm::XFtComm(std::string message): XFt() { msg="XFtComm: "+message; }
XFtComm::~XFtComm() throw() {}
const char* XFtComm::what() const throw() { return msg.c_str(); }

/// communication timeout
XFtCommTimeout::XFtCommTimeout() { msg="XFtCommTimeout"; }
XFtCommTimeout::XFtCommTimeout(std::string message): XFtComm() { msg="XFtCommTimeout: "+message; }
XFtCommTimeout::~XFtCommTimeout() throw() {}
const char* XFtCommTimeout::what() const throw() { return msg.c_str(); }



} // namespace


std::ostream& operator<<(std::ostream& s, const ftapi::IfInfo &x) {
  // save stream settings (we want to switch to hex and back)
  std::ios_base::fmtflags oldflags( s.flags() );

  // print some information
  s<<std::dec<<((ftapi::FT_IF_ROBO==x.ifType)?"Robo Interface":
  ((ftapi::FT_IF_INTELLI==x.ifType)?"Intelligent Interface":
  ((ftapi::FT_IF_IOE==x.ifType)?"Robo I/O Extension":
  ((ftapi::FT_IF_RF_DATA==x.ifType)?"Robo RF Data":
  ((ftapi::FT_IF_SAL==x.ifType)?"Sound+Lights":"--not detected--")))))
  <<" at \""
  <<x.device<<"\" ("<<((ftapi::FT_IC_USB==x.ifConn)?"USB (fischerusb)":
                       (ftapi::FT_IC_LIB_USB==x.ifConn?"USB (libusb)":
                       (ftapi::FT_IC_USB_IOKIT==x.ifConn?"USB (Apple IO-Kit)":
                       (ftapi::FT_IC_SERIAL==x.ifConn?"serial":
                       "unknown"))))<<")";
  if (ftapi::FT_IF_RF_DATA!=x.ifType) {
    s<<std::endl
     <<"interface firmware ver. "
     <<static_cast<int>(x.firmwareVer[0])<<"."
     <<static_cast<int>(x.firmwareVer[1])<<"."
     <<static_cast<int>(x.firmwareVer[2])<<"."
     <<static_cast<int>(x.firmwareVer[3]);
    if (0!=x.serialNo) s
     <<", serial no. "<<std::hex<<x.serialNo<<std::dec;
  }
  if (x.rfdFirmwareVer[0]||x.rfdFirmwareVer[1]||x.rfdFirmwareVer[2]||x.rfdFirmwareVer[3]) s  
   <<std::endl<<"rf data firmware ver. "
   <<static_cast<int>(x.rfdFirmwareVer[0])<<"."
   <<static_cast<int>(x.rfdFirmwareVer[1])<<"."
   <<static_cast<int>(x.rfdFirmwareVer[2])<<"."
   <<static_cast<int>(x.rfdFirmwareVer[3]);
  if (0!=x.rfdSerialNo) s <<", serial no. "<<std::hex<<x.rfdSerialNo<<std::dec;
  if (x.rfdFrequency>0) s
   <<", active radio frequency "<<x.rfdFrequency;
  if ((ftapi::FT_IF_ROBO==x.ifType) 
    ||(ftapi::FT_IF_SAL==x.ifType)) {
    if (x.filenameFlash1.length() > 0) {
      s << std::endl << "Program stored to FLASH1: \"" << x.filenameFlash1 << "\"";
    }
    if (x.filenameFlash2.length() > 0) {
      s << std::endl << "Program stored to FLASH2: \"" << x.filenameFlash2 << "\"";
    }
    if (x.filenameFlash3.length() > 0) {
      s << std::endl << "Program stored to FLASH3: \"" << x.filenameFlash3 << "\"";
    }
    if (x.filenameRam.length() > 0) {
      s << std::endl << "Program stored to RAM: \"" << x.filenameRam << "\"";
    }

  }
    
/* we are unable to detect extensions: 
   the following code does not work properly
  <<std::endl<<"extensions S1:"<<x.slavePresent[0]<<" S1:"
  <<x.slavePresent[1]<<" S2:"<<x.slavePresent[0]
  <<" RadioComm:"<<x.radioCommModulePresent<<" Internet:"<<x.internetModulePresent;*/

  // restore settings
  s.flags(oldflags);
  return s;
}
#endif

// EOF
