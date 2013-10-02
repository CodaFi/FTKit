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
#ifndef FT_INTERFACE_LIB_USB_H
#define FT_INTERFACE_LIB_USB_H
#ifdef HAVE_LIBUSB

#include <usb.h>
#include "ftinterfaceusb.h"
#include "salwavesource.h"


namespace ftapi {

  class FtInterfaceLibUsb: public FtInterfaceUsb {
    private:
    static const int WRITE_TIMEOUT_IN_MS = 300;
    static const int READ_TIMEOUT_IN_MS = 300;

    int rfdGetFrequency();
    int rfdGetRevision();
    void cleanup() throw();
    int getRevision(usb_dev_handle *handle);
    int getTrueSerialNo(usb_dev_handle *handle);
    std::string getFilenameFlash1();
    std::string getFilenameFlash2();
    std::string getFilenameRam();
    std::string getFilenameFlashHelper(unsigned char flashNr);
    bool checkDevSerialNo(int serial, struct usb_device *dev);
    
    IfInfo _info; 
    usb_dev_handle *_handle; /* File descriptor for the port */
    std::string _device;
    bool _rfdConnected;
    
    size_t writeAndReadAllData(unsigned char* outBuf, size_t outSize, unsigned char* inBuf, size_t maxSize, bool verbose=false);


    public:
    FtInterfaceLibUsb(std::string device) throw(XFtComm);
    ~FtInterfaceLibUsb();
    virtual void getIfInfo(IfInfo &info);
    virtual void writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm);
    virtual void startExecution(ft_program_location location);
    virtual void stopExecution();
    virtual bool isExecutionActive(ft_program_location *plocation = 0);
    
    virtual void deleteFlash();
    virtual void storeWavFiles(std::string fn1, std::string fn2, std::string fn3);
  };

} // namespace ftapi
#endif
#endif
