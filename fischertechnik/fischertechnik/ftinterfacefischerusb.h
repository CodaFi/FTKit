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
#ifndef FT_INTERFACE_FISCHER_USB_H
#define FT_INTERFACE_FISCHER_USB_H
#ifdef HAVE_FISCHERUSB

#include "ftinterfaceusb.h"

namespace ftapi {

  class FtInterfaceFischerUsb: public FtInterfaceUsb {
    private:
    int fd; /* File descriptor for the port */
    std::string device;
    IfInfo _info; 

    public:
    FtInterfaceFischerUsb(std::string device) throw(XFtComm);
    ~FtInterfaceFischerUsb();
    virtual void getIfInfo(IfInfo &info);
    virtual void writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm);
    virtual void startExecution(ft_program_location /* location */) { throw XFtComm("not supported"); }
    virtual void stopExecution() { throw XFtComm("not supported"); }
    virtual bool isExecutionActive(ft_program_location* /* plocation = 0 */) { throw XFtComm("not supported"); }
    
    virtual void storeWavFiles(std::string fn1, std::string fn2, std::string fn3) { throw XFtComm("not supported"); }
    virtual void deleteFlash() { throw XFtComm("not supported"); }
   };

} // namespace ftapi
#endif
#endif
