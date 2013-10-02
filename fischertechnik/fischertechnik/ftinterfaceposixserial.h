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
#ifndef FT_INTERFACE_POSIX_SERIAL_H
#define FT_INTERFACE_POSIX_SERIAL_H
#ifdef HAVE_POSIXSERIAL

#include "ftinterfaceserial.h"
#include <iostream>
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */

namespace ftapi {
  class FtInterfacePosixSerial: public FtInterfaceSerial {
    public:
    FtInterfacePosixSerial(std::string device) throw (XFtComm);
    ~FtInterfacePosixSerial();
    
    virtual void getIfInfo(IfInfo &info);
    virtual void writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm);
    virtual void startExecution(ft_program_location /* location */) { throw XFtComm("not supported"); }
    virtual void stopExecution() { throw XFtComm("not supported"); }
    virtual bool isExecutionActive(ft_program_location* /* plocation = 0 */) { throw XFtComm("not supported"); }
    
    virtual void storeWavFiles(std::string /*fn1*/, std::string /*fn2*/, std::string /*fn3*/) { throw XFtComm("not supported"); }
    virtual void deleteFlash() { throw XFtComm("not supported"); }

    
    private:
    void probe() throw (XFtComm);  ///< probe interfaces
    void readAllJunk() throw ();   ///< read bytes from tty as long as new bytes arrive
    unsigned char readByte() throw (XFtCommTimeout); ///< read one byte
    void readSpecialCode(const unsigned char code) throw (XFtCommTimeout); ///< wait for special char
    
    void helperGetInputs(IfInputs &inputs, unsigned char buffer[]);
    
    std::string _device;
    struct termios oldTermIOS;
    struct termios newTermIOS;
    int fd; /* File descriptor for the port */
    static const char INTELLI_IF_REFRESH[];
    static const char INTELLI_IF_QUERY_EX = 0xC6;
    static const char INTELLI_IF_QUERY_EY = 0xCA;
    static const char ROBO_IF_INIT[];
    static const char ROBO_IF_OFF = 0xA2;
    static const char ROBO_IF_COMPLETE = 0xF2;
    static const char ROBO_IF_INTERN_SERIAL[];
    static const char ROBO_IF_INTERN_FIRMWARE[];
    static const int READ_WAIT_US = 100;      // 100us
    static const int READ_TIMEOUT_US = 10000; //  10ms
    
    // constants for extension bits (ABF_IF_COMPLETE)
    static const unsigned char EXT_S1 = 1;
    static const unsigned char EXT_S2 = 2;
    static const unsigned char EXT_S3 = 4;
    static const unsigned char EXT_RADIO_COMM = 16;
    static const unsigned char EXT_INTERNET   = 32;
    
    ft_if_type ifType; ///< interface type, set in ctor
    
    unsigned char robo_if_complete_out[17]; ///< last data block sent completely
    unsigned char robo_if_complete_in[21];  ///< last data block received completely
  };

} // namespace ftapi  
#endif
#endif
// EOF

