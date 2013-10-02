/***************************************************************************
 *  ftApi - fischertechnik c++ progamming api                              *
 *  Copyright (C) 2004,2005 by                                             *
 *  Holger Friedrich <holgerf@vsi.cs.uni-frankfurt.de>,                    *
 *  David Dederscheck <davidded@vsi.cs.uni-frankfurt.de>,                  *
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
#ifndef FT_INTERFACE_USB_APPLE_IO_KIT_H
#define FT_INTERFACE_USB_APPLE_IO_KIT_H
#ifdef HAVE_APPLEIOKIT

#include "ftinterfaceusb.h"
#include <iostream>
#include <fcntl.h>   /* File control definitions */
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <mach/mach.h>

namespace ftapi {
  
  class FtInterfaceUsbAppleIOKit: public FtInterfaceUsb {
    private:
    
#define FTRoboInterfaceUsbVendorID                0x146a
#define FTRoboInterfaceUsbProductID               0x0001
    IOUSBDeviceInterface 	**m_usbDeviceInterface;
    IOUSBInterfaceInterface 	**m_usbInterfaceInterface;
    unsigned char *m_readBuffer;
    unsigned char *m_writeBuffer;
    mach_port_t 		m_masterPort;
    bool m_masterPortAlloced;
    CFMutableDictionaryRef 	m_matchingDict;
    std::string device;
    IfInfo _info; 
    static const char ROBO_IF_COMPLETE = 0xF2;
    static const unsigned int FTRoboInterfaceUsbIOKitReadSize   = 21;
    static const unsigned int FTRoboInterfaceUsbIOKitWriteSize  = 17;
    static const unsigned int FTRoboInterfaceWriteEndpoint      =	3;
    static const unsigned int FTRoboInterfaceReadEndpoint       =	1;
    virtual void initVar();
    virtual void cleanup();
    public:
    FtInterfaceUsbAppleIOKit(std::string device) throw(XFtComm);
    ~FtInterfaceUsbAppleIOKit();
    
    virtual void getIfInfo(IfInfo &info);
    virtual void writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm);
    virtual void startExecution(ft_program_location location) { throw XFtComm("not supported"); }
    virtual void stopExecution() { throw XFtComm("not supported"); }
    virtual bool isExecutionActive(ft_program_location *plocation = 0) { throw XFtComm("not supported"); }    
    

    virtual void storeWavFiles(std::string fn1, std::string fn2, std::string fn3) { throw XFtComm("not supported"); }
    virtual void deleteFlash() { throw XFtComm("not supported"); }

    
    // constants for extension bits
    static const unsigned char EXT_S1 = 1;
    static const unsigned char EXT_S2 = 2;
    static const unsigned char EXT_S3 = 4;
    static const unsigned char EXT_RADIO_COMM = 16;
    static const unsigned char EXT_INTERNET   = 32;
  };

} // namespace ftapi
#endif
#endif
