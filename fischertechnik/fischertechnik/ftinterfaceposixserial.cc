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
#ifdef HAVE_POSIXSERIAL
#include "ftinterfaceposixserial.h"
#include <fcntl.h>   /* File control definitions */
#include <iostream>
#include <sstream>
#include <errno.h>   /* EAGAIN, etc. */
#include <cassert>
#include <strings.h>


/** @file
implementation */
namespace ftapi {


const char FtInterfacePosixSerial::ROBO_IF_INIT[]={0xA1,'f','t','-','R','o','b','o','-','O','N','-','V','1','\n',0};
const char FtInterfacePosixSerial::INTELLI_IF_REFRESH[]={0xD2,0x0A};
const char FtInterfacePosixSerial::ROBO_IF_INTERN_FIRMWARE[]={0xF0,0x01};
const char FtInterfacePosixSerial::ROBO_IF_INTERN_SERIAL[]={0xF0,0x02};


void FtInterfacePosixSerial::getIfInfo(IfInfo &info) {
  info = IfInfo(); // new, all members cleared
  info.ifType = ifType;
  info.ifConn = FT_IC_SERIAL;
  info.device = _device;
  if (FT_IF_ROBO==ifType) { // gather more info
    // firmware
    int n = write(fd, ROBO_IF_INTERN_FIRMWARE, 2);
    if (n < 0)
      throw XFtComm("FtInterfacePosixSerial: write() of 2 bytes failed!");
    readSpecialCode(~(ROBO_IF_INTERN_FIRMWARE[1]));
    for (n=0; n<4; n++)
      info.firmwareVer[3-n]=readByte();
    
    // serial no (currently working!)
    n = write(fd, ROBO_IF_INTERN_SERIAL, 2);    
    if (n < 0)
      throw XFtComm("FtInterfacePosixSerial: write() of 2 bytes failed!");
    try {
      readSpecialCode(~(ROBO_IF_INTERN_SERIAL[1]));
//for (n=1;n<200;n++) 
//  std::cout << (int)readByte() << std::endl;
      // NOTE: this is _not_ according to spec!
      // there will be 14 bytes:
      // 1 byte special code ~(ROBO_IF_INTERN_SERIAL[1])
      // 4 bytes hard coded serial ?
      // 4 bytes standard serial (1)
      // 4 bytes hard coded serial
      // 1 byte which one is active?
      for (n=0; n<8; n++) readByte();
      int ser = 0;
      for (n=0; n<4; n++) {
         ser |= static_cast<int>(readByte()) << (n*8);
      }
      readByte();
      info.serialNo = ser;
    } catch (XFtComm &x) {
      info.serialNo = 0;
    }
  
    // extension modules (write, read, extract info)
    n = write(fd, robo_if_complete_out, 17); // use last state (does not change anything)
    if (n < 0)
      throw XFtComm("FtInterfacePosixSerial: write() of 17 bytes failed!");
    unsigned char buffer[21];
    for (n=0; n<21; n++)
     buffer[n]=readByte();
    // received complete, save
    for (n=0; n<21; n++)
     robo_if_complete_in[n] = buffer[n];
/* we are unable to detect extensions: 
   the following code does not work properly
    const unsigned char extensions = buffer[15];
    info.slavePresent[0] = ((extensions & EXT_S1)!=0);
    info.slavePresent[1] = ((extensions & EXT_S2)!=0);
    info.slavePresent[2] = ((extensions & EXT_S3)!=0);
    info.radioCommModulePresent = ((extensions & EXT_RADIO_COMM)!=0);
    info.internetModulePresent  = ((extensions & EXT_INTERNET)!=0);
*/
  }
}

void FtInterfacePosixSerial::helperGetInputs(IfInputs &inputs, unsigned char buffer[]) {
  assert(NULL != buffer); // normally use throw, but function is internal, caller must check
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
  // TODO ask for meaning of this bytes!
  inputs.irKey           = buffer[14+17] & 15;
  inputs.code2           = (buffer[14+17] >> 4) & 1;
  inputs.as1Digital      = (buffer[14+17] >> 6) & 1;
  inputs.as2Digital      = (buffer[14+17] >> 7) & 1;
  // installed modules     buffer[15] (not to implement here)
  inputs.timeInMs        = buffer[16];
  inputs.ax[FT_SLAVE1]   = buffer[17] + (static_cast<int>((buffer[20]   )&3)<<8);
  inputs.ax[FT_SLAVE2]   = buffer[18] + (static_cast<int>((buffer[20]>>2)&3)<<8);
  inputs.ax[FT_SLAVE3]   = buffer[19] + (static_cast<int>((buffer[20]>>4)&3)<<8);
}
    
void FtInterfacePosixSerial::writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm) {
  switch (ifType) {
    case FT_IF_ROBO: {
      // build a new set of instructions (outputs -> buffer)
      unsigned char buffer[21]; // 21=max( 17(out),21(in) )
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
          
      int n = write(fd, buffer, 17);
      if (n < 0)
        throw XFtComm("FtInterfacePosixSerial: write() of 17 bytes failed!");
      // seems to be ok, copy buffer to save last write
      for (n=0; n<17; n++)
      robo_if_complete_out[n] = buffer[n];
      // read
      for (n=0; n<21; n++)
      buffer[n]=readByte();
      // received complete, save as last read
      for (n=0; n<21; n++)
      robo_if_complete_in[n] = buffer[n];
      
      // buffer -> inputs
      helperGetInputs(inputs, buffer);
      }
      break;
    case FT_IF_INTELLI: {
      // Intelligent Interface Art.-Nr.30402 communication is documented
      // at fischertechnik homepage

      // clear all inputs -> done by ctor
      
      // prepare motor byte
      unsigned char motors = 0, motorm = 0;

      for (int i=3; i>=0; --i) {
        motorm = motorm << 2;
        motors = motors << 2;
        if (outputs.speed[FT_MASTER][2*i] > outputs.speed[FT_MASTER][2*i+1]) {
           motorm |= 1;
        } else if (outputs.speed[FT_MASTER][2*i] < outputs.speed[FT_MASTER][2*i+1]) {
           motorm |= 2;
        } // else =, do nothing
        if (outputs.speed[FT_SLAVE1][2*i] > outputs.speed[FT_SLAVE1][2*i+1]) {
           motors |= 1;
        } else if (outputs.speed[FT_SLAVE1][2*i] < outputs.speed[FT_SLAVE1][2*i+1]) {
           motors |= 2;
        } // else =, do nothing
      }

      // set motors, query EX
      unsigned char buffer[3];
      buffer[0] = INTELLI_IF_QUERY_EX;
      buffer[1] = motorm;
      buffer[2] = motors;
      int n = write(fd, buffer, 3);
      if (n < 3)
        throw XFtComm("FtInterfacePosixSerial: write() of 3 bytes failed!");
      int tmp;
      tmp = readByte(); // read and discard digital inputs (see below)
      tmp = readByte(); // read and discard digital inputs (see below)
      tmp = readByte();
      tmp = tmp << 8;
      tmp |= readByte();
      inputs.ax[FT_MASTER] = tmp;
      // same again, but query EY 
      buffer[0] = INTELLI_IF_QUERY_EY;
      buffer[1] = motorm;
      buffer[2] = motors;
      n = write(fd, buffer, 3);
      if (n < 3)
        throw XFtComm("FtInterfacePosixSerial: write() of 3 bytes failed!");
      inputs.data[FT_MASTER] = readByte(); // read and save inputs
      inputs.data[FT_SLAVE1] = readByte(); // read and save inputs
      tmp = readByte();
      tmp = tmp << 8;
      tmp |= readByte();
      inputs.ay = tmp;
       }
      break;
    default:
      throw XFtComm("FtInterfacePosixSerial: unknown interface");
  } // switch
}



FtInterfacePosixSerial::FtInterfacePosixSerial(std::string device) throw(XFtComm)
: _device(device) {
  fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
   /*
    * Could not open the port.
    */
    throw XFtComm("FtInterfacePosixSerial: unable to open tty");
  } else {
    // ok, port opened
    // now probe for RoboIF and IntelligentIF
    probe();
    
    if (FT_IF_UNKNOWN==ifType) throw XFtComm("FtInterfacePosixSerial: unable to detect Interface");
    
    bzero(robo_if_complete_out, sizeof(robo_if_complete_out));
    robo_if_complete_out[0]=ROBO_IF_COMPLETE;
    bzero(robo_if_complete_in, sizeof(robo_if_complete_in));
  }
}

FtInterfacePosixSerial::~FtInterfacePosixSerial() {
  if (FT_IF_ROBO == ifType) {
    // deactivate device
    unsigned char buf[1];
    buf[0] = ROBO_IF_OFF;
    int n = write(fd, buf, 1);
    if (n == 1) {
      try{ 
        n = ~ROBO_IF_OFF;
        readSpecialCode(n);
      } catch (...) {
	    } // dtors must _not_ throw
      // ok, thats it
    } // else: error, but dtors must _not_ throw 
  }
  tcsetattr(fd, TCSAFLUSH, &oldTermIOS);  // restore old settings, do not care if it works
  close(fd);                      // close port, do not care if it works
}


void FtInterfacePosixSerial::probe() throw (XFtComm) {
  ifType = FT_IF_UNKNOWN;
  
  fcntl(fd, F_SETFL, FNDELAY);        // non blocking mode
  
  tcgetattr(fd, &oldTermIOS);   // to be restored in later...
  tcgetattr(fd, &newTermIOS);   // get the current settings
  // raw terminal
  cfmakeraw(&newTermIOS);       // make raw struct (see man 3 termios)
  // baud rate
  cfsetospeed(&newTermIOS, B38400);   // set baud rate to 38400 (for robo if)
  cfsetispeed(&newTermIOS, B38400);       // set in baud rate := out baud rate
  // 8,n,1
  newTermIOS.c_cflag &= ~PARENB;
  newTermIOS.c_cflag &= ~CSTOPB;
  newTermIOS.c_cflag &= ~CSIZE;
  newTermIOS.c_cflag |= CS8;
  // do not wait for line endings!
  newTermIOS.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  tcsetattr(fd, 0, &newTermIOS);      // invoke new settings
  
  readAllJunk(); 

  int n;
  try { 
    n = write(fd, ROBO_IF_INIT, 14);
    if (n < 0)
      throw XFtComm("FtInterfacePosixSerial: write() of 4 bytes failed!");
    
    readSpecialCode(~(ROBO_IF_INIT[0]));
    readByte(); // firmware F.W.B.L.
    readByte(); // firmware
    readByte(); // firmware
    readByte(); // firmware
    
    ifType = FT_IF_ROBO;
  } catch (XFtCommTimeout &x) {
    // Robo Interface NOT detected, maybe Intelligent Interface is present...
    cfsetospeed(&newTermIOS, B9600);   // set baud rate to 38400 (for robo if)
    cfsetispeed(&newTermIOS, B9600);   // set in baud rate := out baud rate
    tcsetattr(fd, 0, &newTermIOS);      // invoke new settings

    // set motors, query EX
    readAllJunk();
    unsigned char buffer[3];
    buffer[0] = INTELLI_IF_QUERY_EX;
    buffer[1] = 0;
    buffer[2] = 0;
    int n = write(fd, buffer, 3);
    if (n < 3)
      throw XFtComm("FtInterfacePosixSerial: write() of 3 bytes failed!");
    int tmp;
    tmp = readByte(); // read and discard digital inputs (see below)
    tmp = readByte(); // read and discard digital inputs (see below)
    tmp = readByte(); // read and discard digital inputs (see below)
    tmp = readByte(); // read and discard digital inputs (see below)
 
    try {
      tmp = readByte(); // should _not_ work, exactly 4 bytes expected! 
    } catch (XFtComm &x) {  
      ifType = FT_IF_INTELLI; // else exception is thrown -> function exit
    }
  }
  
  // ok, now we have clarified interface type
}

void FtInterfacePosixSerial::readAllJunk() throw () {
  bool enough=false;
  int b;
  unsigned char c;
  while (!enough) {
    while(1==(b=read(fd,&c,1))) ;            // read junk
    usleep(20);                              // wait for a small time
    if (1>(b=read(fd,&c,1))) enough = true;  // see if more bytes come in
  }
}

unsigned char FtInterfacePosixSerial::readByte() throw (XFtCommTimeout) {
  unsigned char c=0;
  int b;
  int waitCount=0;
  while(1>(b=read(fd,&c,1))) { // read byte
    waitCount+=READ_WAIT_US;
    if (READ_TIMEOUT_US<waitCount) throw XFtCommTimeout();
    usleep(READ_WAIT_US); 
  }
  return c;
}


void FtInterfacePosixSerial::readSpecialCode(const unsigned char code) throw (XFtCommTimeout) {
  bool enough=false;
  int b;
  unsigned char c;
  int waitCount=0;
  while (!enough) {
    ((b=read(fd,&c,1))) ;
    if (b==1) {
      if (code==c) // inverted command code
        enough=true;
    } else {
      waitCount+=READ_WAIT_US;
      if (READ_TIMEOUT_US<waitCount) throw XFtCommTimeout();
      usleep(READ_WAIT_US);
    }
  }
}

} // namespace
#endif
// EOF
