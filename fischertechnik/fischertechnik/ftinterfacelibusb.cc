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
#ifdef HAVE_LIBUSB
#include "ftinterfacelibusb.h"
#include "ftinternal.h"
#include <iostream>
#include <sstream>
#include <cassert>
//#include <strings.h>
#include <string.h>
#include <algorithm>
#include <iomanip>


/** @file
implementation */
namespace ftapi {

/* query robo rf data's radio frequency (2..80) */
int FtInterfaceLibUsb::rfdGetFrequency() {
  if (NULL == _handle) throw XFtComm("cannot read radio channel h");
 
  unsigned char buffer[15];
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_RFD_SPECIAL, ROBO_RFD_INTERN_REV, 
   0, reinterpret_cast<char*>(buffer), 
   8, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  //for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 

  unsigned char c = ~ROBO_RFD_INTERN_REV;
  if ((8!=res) || (c != (buffer[0]))) { // error
    throw XFtComm("cannot read radio channel");
  }

  // check, if it is really there (if yes, it should have serial)
  if (buffer[1]||buffer[2]||buffer[3]||buffer[4]) return buffer[6];  
  return 0;
}

/* query revision number. format is firmware/bootloader: f.w.b.l. */
int FtInterfaceLibUsb::rfdGetRevision() {
  if (NULL == _handle) throw XFtComm("cannot read rf date firmware revision");
 
  unsigned char buffer[15];
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_RFD_SPECIAL, ROBO_RFD_INTERN_REV, 
   0, reinterpret_cast<char*>(buffer), 
   8, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);

  unsigned char c = ~ROBO_RFD_INTERN_REV;
  if ((8!=res) || (c != (buffer[0]))) { // error
    throw XFtComm("cannot read rf data firmware revision");
  }

  int i = buffer[4];
  i = (i<<8) | buffer[3];
  i = (i<<8) | buffer[2];
  i = (i<<8) | buffer[1];
  return i;  
}

/* query robo rf data's cleanup function (e.g. turn of radio sender) */
void FtInterfaceLibUsb::cleanup() throw() {
   if (FT_IF_RF_DATA == _info.ifType) {
    // RF DATA: turn off RF sender
    // code is still EXPERIMENTAL!
    unsigned char buffer[15];
    memset(buffer, 0, sizeof(buffer));
    usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
     ROBO_RFD_RF_OFF, ROBO_RFD_RF, 0, reinterpret_cast<char*>(buffer), 
     5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    // for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 
    // no error handling since we can not throw in dtor
  }
  if (0 != _handle) {
    usb_release_interface(_handle, 0);
    usb_close(_handle);
    _handle = 0;
  }
}

/* query revision number. format is firmware/bootloader: f.w.b.l. */
int FtInterfaceLibUsb::getRevision(usb_dev_handle *handle) {
  if (NULL == handle) throw XFtComm("cannot read revision");
 
  unsigned char buffer[15];
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, ROBO_IF_INTERN_REV, 0, reinterpret_cast<char*>(buffer), 
   5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);

  unsigned char c = ~ROBO_IF_INTERN_REV;
  if ((5 != res) || (c != (buffer[0]))) { // error
    throw XFtComm("cannot read revision");
  }

  int i = buffer[4];
  i = (i<<8) | buffer[3];
  i = (i<<8) | buffer[2];
  i = (i<<8) | buffer[1];
  return i;  
}

/* query serial printed on device.
every device has a so called "standard serial" which seems
to be always "1" and the serial printed on the device.
@note dev must be opened (ok, this is the case, else you
could not give me an handle) and - for win32 systems - the
configuration must be set! */
int FtInterfaceLibUsb::getTrueSerialNo(usb_dev_handle *handle) {
  if (NULL == handle) throw XFtComm("cannot read serial number");
 
  unsigned char buffer[15];
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, ROBO_IF_INTERN_SN, 0, reinterpret_cast<char*>(buffer), 
   14, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  // for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 
 
  unsigned char c = ~ROBO_IF_INTERN_SN;
  if ((14 != res) || (c != (buffer[0]))) { // error
    throw XFtComm("cannot read serial number");
  }

  int i = buffer[12];
  i = (i<<8) | buffer[11];
  i = (i<<8) | buffer[10];
  i = (i<<8) | buffer[9];
  return i;  
}

/* check if serial of usb device equals string.
special strings are allowed: empty or "*" matches everything */
bool FtInterfaceLibUsb::checkDevSerialNo(int serial, struct usb_device *dev) {
  if (NULL == dev) return false;

  try {
    // ...else probe!
    usb_dev_handle *handle; /* File descriptor for the port */
    handle = usb_open(dev);
    if (NULL == handle) throw XFtComm("FtInterfaceLibUsb: cannot open device");

    int res = 0;

#ifdef WIN32 
    /* device must not be configured on MacOSX as it is a composite device
      which is claimed by the apple composited driver */
    res = usb_set_configuration(handle, 1);
    if (0 != res) {
      usb_close(handle);
      throw XFtComm("FtInterfaceLibUsb: cannot set configuration");
    }
#endif

    res = usb_claim_interface(handle, 0);
    if (0 != res) {
      usb_close(handle);
      throw XFtComm("FtInterfaceLibUsb: cannot claim interface");
    }

    try {
      if (getTrueSerialNo(handle) == serial) {
        usb_release_interface(handle, 0);
        usb_close(handle);
        return true; // match!
      }
    } catch (...) {}
    usb_release_interface(handle, 0);
    usb_close(handle);
  } catch (...) {}
  return false;
}


std::string FtInterfaceLibUsb::getFilenameFlash1() {
  return getFilenameFlashHelper(0);
}


std::string FtInterfaceLibUsb::getFilenameFlash2() {
  return getFilenameFlashHelper(1);
}


std::string FtInterfaceLibUsb::getFilenameRam() {
  return getFilenameFlashHelper(2);
}


std::string FtInterfaceLibUsb::getFilenameFlashHelper(unsigned char flashNr) {
  assert(flashNr < 3); // 0: flash1, 1: flash2, 2: ram
  static const int bufferLength = 0x51;
  unsigned char buffer[bufferLength + 1];
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_FLASH_QUERYNAME, flashNr, 0, reinterpret_cast<char*>(buffer), 
   bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  
  if (bufferLength != res) throw XFtComm("cannot read filename from FLASH");
  // actually we do not know what buffer[0] indicates
  //std::cerr << "b[0]: " << std::hex << (int)buffer[0] << std::endl;
  buffer[bufferLength] = 0; // now it is zero-terminated
  std::string result( reinterpret_cast<char*>( &(buffer[1]) ) );
  return result;
}


FtInterfaceLibUsb::FtInterfaceLibUsb(std::string device) throw(XFtComm): 
 _device(device), _rfdConnected(false) {
  struct usb_bus *busses;
  
  usb_init();
  usb_find_busses();
  usb_find_devices();
  
  // only for debugging:
#ifndef NDEBUG
  usb_set_debug(0xFF);
#endif
  
  busses = usb_get_busses();
  if (NULL == busses) throw XFtComm("FtInterfaceLibUsb: unable to get usb bus list");
  
  struct usb_bus *bus;
    
  /* ... */
  struct usb_device *mydev = NULL;
  bool acceptAnySerialNo(false);
  int deviceSerialNo(0);
  if ( (0 == device.length()) || (device == std::string("*")) ) {
    acceptAnySerialNo = true;
  } else { // parse string
    std::istringstream serialStream(std::string("0x") + _device); 
    serialStream >> std::hex >> deviceSerialNo;
    if (!serialStream.eof()) throw // string could not be parsed completely
     XFtComm(std::string("\"") + _device + std::string("\" is not a valid serial number")); 
  }
    
  for (bus = busses; bus; bus = bus->next) {
    struct usb_device *dev;
    for (dev = bus->devices; NULL != dev; dev = dev->next) {
      if ( ((ROBO_USB_VENDOR_ID == dev->descriptor.idVendor)
         && (ROBO_IF_USB_PRODUCT_ID == dev->descriptor.idProduct)) ||
           ((ROBO_USB_VENDOR_ID == dev->descriptor.idVendor)
         && (ROBO_IOE_USB_PRODUCT_ID == dev->descriptor.idProduct)) ||
           ((ROBO_USB_VENDOR_ID == dev->descriptor.idVendor)
         && (ROBO_RFD_USB_PRODUCT_ID == dev->descriptor.idProduct)) ||
           ((ROBO_USB_VENDOR_ID == dev->descriptor.idVendor)
         && (ROBO_SAL_USB_PRODUCT_ID == dev->descriptor.idProduct))
          ) { 
        if (acceptAnySerialNo || checkDevSerialNo(deviceSerialNo, dev)) {
          mydev = dev;
          dev->next = NULL; // force quit loop
        }
      }
    }
  }
  if (NULL == mydev) throw XFtComm("FtInterfaceLibUsb: cannot find device");

  // ...else probe!
  _handle = usb_open(mydev);
  if (NULL == _handle) throw XFtComm("FtInterfaceLibUsb: cannot open device");

  int res = 0;
  
#ifdef WIN32
  /* device must not be configured on MacOSX as it is a composite device
    which is claimed by the apple composited driver */
  res = usb_set_configuration(_handle, 1);
  if (0 != res) throw XFtComm("FtInterfaceLibUsb: cannot set configuration");
#endif        

  res = usb_claim_interface(_handle, 0);
  if (0 != res) throw XFtComm("FtInterfaceLibUsb: cannot claim interface");

  unsigned char buffer[28];
  memset(buffer, 0, sizeof(buffer));
  int tries=0;
  // I know, the following is dirty (but currently prod ids are unique)
  switch(mydev->descriptor.idProduct) {
    case ROBO_IF_USB_PRODUCT_ID:
      _info.ifType = FT_IF_ROBO;
      // try to send one packet and read response...
      for (int i=0; i<28; ++i) { buffer[i] = 0; }
      buffer[0] = ROBO_IF_COMPLETE;
      do {
        res = usb_interrupt_write(_handle, ROBO_IF_OUT_EP, 
        reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_WRITE_SIZE, 
        WRITE_TIMEOUT_IN_MS);
        if (ROBO_IF_COMPLETE_WRITE_SIZE != res) {
          cleanup();
          throw XFtComm("FtInterfaceLibUsb: cannot send");
        }
        res = usb_interrupt_read(_handle, ROBO_IF_IN_EP, 
        reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_READ_SIZE,
        (READ_TIMEOUT_IN_MS/10)+1);
        if ((ROBO_IF_COMPLETE_READ_SIZE != res) && (tries>10)) {
          cleanup();
          throw XFtComm("FtInterfaceLibUsb: cannot receive");
        }
        if (ROBO_IF_COMPLETE_READ_SIZE==res) break;
        ++tries;
      } while (1);
      
      
      // seems to work
      _info.device = device;
      _info.ifConn = FT_IC_LIB_USB;
      break;
  
    ///////////////////// IO EXT //////////////////////////// 
    case ROBO_IOE_USB_PRODUCT_ID:
      _info.ifType = FT_IF_IOE;
      for (int i=0; i<28; ++i) { buffer[i] = 0; }
      buffer[0] = ROBO_IF_COMPLETE;
      buffer[5] = ROBO_IOE_MARKING;
  
      res = usb_interrupt_write(_handle, ROBO_IF_OUT_EP, 
       reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_WRITE_SIZE, 
       WRITE_TIMEOUT_IN_MS);
      if (res != ROBO_IF_COMPLETE_WRITE_SIZE) {
        cleanup();
        throw XFtComm("FtInterfaceLibUsb: cannot send");
      }
  
      res = usb_interrupt_read(_handle, ROBO_IF_IN_EP, 
       reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_READ_SIZE,
       READ_TIMEOUT_IN_MS);
      if (res != ROBO_IOE_COMPLETE_READ_SIZE) {
        cleanup();
        throw XFtComm("FtInterfaceLibUsb: cannot receive");
      } 
      // seems to work
      _info.device = device;
      _info.ifConn = FT_IC_LIB_USB;
    break;

    ///////////////////// RFDATA //////////////////////////// 
    case ROBO_RFD_USB_PRODUCT_ID:
      _info.ifType = FT_IF_RF_DATA;
      for (int i=0; i<28; ++i) { buffer[i] = 0; }

      // SUPPORT FOR ROBO RF DATA IS STILL EXPERIMENTIAL!

/*   for (int cnt=0; cnt<3; cnt++) {
     res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
     0x50, 0x103, 0, reinterpret_cast<char*>(buffer), 
     5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
     for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 
   }

   res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   0xFB, 0x81, 0, reinterpret_cast<char*>(buffer), 
   5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 

   res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   0xFB, 0x82, 0, reinterpret_cast<char*>(buffer), 
   5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 

     res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
     0x50, 0x103, 0, reinterpret_cast<char*>(buffer), 
     5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
     for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; */

      // turn radio sender on
      res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
       ROBO_RFD_RF_ON, ROBO_RFD_RF, 1, 
       reinterpret_cast<char*>(buffer), 5, 
       WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);

      // for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 

      if (2 != res) throw XFtComm("FtInterfaceLibUsb: could not activate RF sender");
      if ( (ROBO_RFD_RF_ON+2) != buffer[0] ) throw 
       XFtComm("FtInterfaceLibUsb: could not activate RF sender");

      if (0 == buffer[1]) {
        // connection to robo if has been established
        _rfdConnected = true;
        // since dtor will not be called when ctor fails we must stop RF sender
        //usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
        // ROBO_RFD_RF_OFF, ROBO_RFD_RF, 0, reinterpret_cast<char*>(buffer), 
        // 5, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
        //throw XFtComm("FtInterfaceLibUsb: could not connect to robo interface");
      }


//      std::cout << "RFD: " << usb_control_msg(_handle, 0xF0, 2, 0, 0, reinterpret_cast<char*>(buffer), 5, WRITE_TIMEOUT_IN_MS) << std::endl; 
//      std::cout << "RFD: " << usb_control_msg(_handle, 0x50, 0x103, 0, 0, reinterpret_cast<char*>(buffer), 5, WRITE_TIMEOUT_IN_MS) << std::endl; 

      for (int i=0; i<28; ++i) { buffer[i] = 0; }
      buffer[0] = ROBO_RFD_IF_COMPLETE;
 
      do { 
        res = usb_interrupt_write(_handle, ROBO_RFD_IF_OUT_EP, 
         reinterpret_cast<char*>(&(buffer[0])), ROBO_RFD_IF_COMPLETE_WRITE_SIZE, 
         WRITE_TIMEOUT_IN_MS);
        if (ROBO_RFD_IF_COMPLETE_WRITE_SIZE != res) {
          cleanup();
          throw XFtComm("FtInterfaceLibUsb: cannot send");
        }
        res = usb_interrupt_read(_handle, ROBO_RFD_IF_IN_EP, 
         reinterpret_cast<char*>(&(buffer[0])), ROBO_RFD_IF_COMPLETE_READ_SIZE,
         READ_TIMEOUT_IN_MS);
        if ( (ROBO_RFD_IF_COMPLETE_READ_SIZE != res) && (tries>10) ) {
          cleanup();
          throw XFtComm("FtInterfaceLibUsb: cannot receive");
        }
        if (ROBO_RFD_IF_COMPLETE_READ_SIZE==res) break;
        ++tries;
      } while (1);
      // seems to work
      _info.device = device;
      _info.ifConn = FT_IC_LIB_USB;
      break;

    case ROBO_SAL_USB_PRODUCT_ID:
    
#ifndef NDEBUG
{/* // unknown vendor call: 32. always returns 1.
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, 32, 0, reinterpret_cast<char*>(buffer), 
   sizeof(buffer)-1, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   std::cout << "SAL vendor call #32, received " << res << " bytes" << std::endl;
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; */
/* #32 rec 1
1
*/    
}         
{ // unknown vendor call: 40. always returns 1.
/*  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, 40, 0, reinterpret_cast<char*>(buffer), 
   sizeof(buffer)-1, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   std::cout << "SAL vendor call #40, received " << res <<  " bytes" << std::endl;
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; */
/* #40 rec 1
1
*/    
}    
{ // unknown vencor call: ROBO_IF_INTERN_MODE
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, ROBO_IF_INTERN_MODE, 0, reinterpret_cast<char*>(buffer), 
   sizeof(buffer)-1, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   std::cout << "SAL vendor call ROBO_IF_INTERN_MODE, received " << res 
     << " bytes:" << std::endl;   
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << " "; 
   if (res>0) std::cout << std::endl;
/*
03: rec 10
252
0
156
0
0
1
0
0
0
133

or: (after doing something with ep bulk write)
252
0
156
0
48
0
48
0
48
0
*/   
}
{
  char buf[500];
  for (char no = 0; no<3; ++no) {
    unsigned char msg[16] = {0xBC,1,0,no, 0,no+1,6,7, 8,9,0xA,0xB, 0xC,0xD,0xE,no+1};
    usb_clear_halt(_handle, 1);
      int res = usb_bulk_write(_handle, USB_ENDPOINT_OUT + 1, (char*)msg, sizeof(msg), WRITE_TIMEOUT_IN_MS);
      // std::cout << "fn #" << (int)no << " wrote " << res << " bytes" << std::endl;
     memset(buf, 0, sizeof(buf));
    tries = 40;
    do {
      res = usb_bulk_read(_handle, USB_ENDPOINT_IN + 1, 
      reinterpret_cast<char*>(buf), 
      sizeof(buf)-1, READ_TIMEOUT_IN_MS*10);
#ifndef NDEBUG    
      if (1) { // packets #0 and #1 are nearly completely understood by now
        std::cout << "SAL bulk read, received " << res << " bytes:";
        for (int j=0; j<res; j++) {
          unsigned char c = buf[j];
          if (j%18==0) std::cout << std::endl;
          std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)c << " "; 
        }
        std::cout << std::endl;
      }
#endif
      --tries;
    } while ( (res<=0) && (tries) );
    if (!tries) throw XFtComm("could not read from Sound+Lights module");
    
    // msg #0 contains length in bytes
    if (0==no) {
      if ((0xf0!=(unsigned char)buf[0])||(0!=buf[1])||(0x17!=buf[2])) throw XFtComm("unexpected package data 1");
      unsigned int blocks;
      blocks = (unsigned char)buf[3];
      blocks = blocks << 8;
      blocks |= (unsigned char)buf[4];
      static const unsigned int blks = 0x07fd;
      std::cout << "free space: " << std::fixed << std::setprecision(1) << blocks * 256.0 / 22000.0
        << "s ("
        << std::dec << blocks << "/" << blks << " blocks; " << std::fixed << std::setprecision(0)
        << 100.0*(float)(blks-blocks)/(float)blks << "% used)" << std::endl;

      unsigned int blen, plen, badr;
      for (int j=0; j<3; ++j) {
        size_t base = 29+j*20;
        
        if ((j!=buf[base])&&(0xff!=(unsigned char)buf[base])) throw XFtComm("unexpected package data 2");

        plen = (unsigned char)buf[base+1+0];
        plen = plen << 8;
        plen |= (unsigned char)buf[base+1+1];

        badr = (unsigned char)buf[base+3+0];
        badr = badr << 8;
        badr |= (unsigned char)buf[base+3+1];

        blen = (unsigned char)buf[base+5+0];
        //std::cout << "base: " << std::hex << blen << std::endl;
        blen = blen << 8;
        blen |= (unsigned char)buf[base+5+1];
        //std::cout << "base: " << std::hex << blen << std::endl;
        blen = blen << 8;
        blen |= (unsigned char)buf[base+5+2];
        //std::cout << "base: " << std::hex << blen << std::endl;
        blen = blen << 8;
        blen |= (unsigned char)buf[base+5+3];
        //std::cout << "base: " << std::hex << blen << std::endl;
      
        if (0xff == (unsigned char)buf[base]) {
          if ((0!=plen)||(0!=blen)||(0!=badr)) throw XFtComm("unexpected package data 3");
        }

        float slenf = blen / 22000.0;  // 22kHz
        std::cout << "#" << j << ": " << std::fixed << std::setprecision(1) << slenf << "s wav, " << std::dec << 
          blen << " blocks starting at block adress 0x" << std::setw(4) << std::hex << badr << std::endl;
      }
    }
    
    // msg #1 contains the names of the sound files stored to SAL module
    if (1==no) {
      if ((0xf1!=(unsigned char)buf[0])||(0!=buf[1])||(1!=buf[2])) throw XFtComm("unexpected package data 4");
#ifndef NDEBUG    
      using namespace std; // min/max are in std (linux), but not in msvc
      std::cout << "SAL bulk read, received " << res << " bytes:" << std::endl;
      for (int j=0; j<min(res,8); j++) {
        unsigned char c = buf[j];
        // if (j%18==0) std::cout << std::endl;
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)c << " "; 
      }
      if (res>0) std::cout << "... [3xfilename] ... ";
      for (int j=8+80*3; j<max(res,8); j++) {
        unsigned char c = buf[j];
        // if (j%18==0) std::cout << std::endl;
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)c << " "; 
      }
      if (res>0) std::cout << std::endl;
#endif
      for (int j=0; j<3; ++j) {
        int idx = 8+80*j;
        std::ostringstream fn;
        while ((idx < res) && (idx<8+80*(j+1)) && (buf[idx]!=0)) {
          fn << buf[idx];
          ++idx;
        }
        std::ostringstream empty1;
        std::ostringstream empty2;
        for (int k=0; k<80; ++k) {
          empty1 << ' ';          // deleted cleanly: 0x32
          empty2 << (char)(-1);   // crashed during update: -1
        }
        if (fn.str() == empty1.str()) fn.str("");
        if (fn.str() == empty2.str()) fn.str("");
        switch (j) {
          case 0: _info.filenameFlash1 = fn.str(); break;
          case 1: _info.filenameFlash2 = fn.str(); break;
          case 2: _info.filenameFlash3 = fn.str(); break;
          default: assert(1);
        }
        //std::cout << "fn #" << (int)j << ": \"" << fn.str() << "\"" << std::endl;
      }
    } // filenames
    
    if (3==no) {
      if ((0xf1!=(unsigned char)buf[0])||(0!=buf[1])||(2!=buf[2])) throw XFtComm("unexpected package data");
    }

  }
}

#endif
#if 0
{ // knwon call (fw rev)
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, ROBO_IF_INTERN_REV, 0, reinterpret_cast<char*>(buffer), 
   sizeof(buffer)-1, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   std::cout << "ROBO_IF_INTERN_REV rec " << res << std::endl;
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 
   
/*
01: rec 5
254
1
0
23
0
*/   
   
}   
    
    
{ // known call: serial (current/saved + real)
  memset(buffer, 0, sizeof(buffer));
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_INTERN, ROBO_IF_INTERN_SN, 0, reinterpret_cast<char*>(buffer), 
   sizeof(buffer)-1, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
   std::cout << "ROBO_IF_INTERN_SN rec " << res << std::endl;
   for (int j=0; j<res; j++) std::cout << (int)buffer[j] << std::endl; 
   
/* rec 14
253
1
0
0
0
1
0
0
0
133
18
1
0
1
*/
}
#endif
      _info.device = device;
      _info.ifConn = FT_IC_LIB_USB;
      _info.ifType = FT_IF_SAL;
      
      break;

    default:
      throw XFtComm("FtInterfaceLibUsb: unknown prod. id");
  }
  // serial no
  try {
    if (FT_IF_RF_DATA != _info.ifType) {
      _info.serialNo = getTrueSerialNo(_handle);
      _info.rfdSerialNo = 0; // not used
    } else { // RF DATA
      _info.serialNo = 0; // FIXME we can not detect it yet
      _info.rfdSerialNo = getTrueSerialNo(_handle);
    }
  } catch (...) {
    cleanup();
    throw;
  }
  // revision
  int bcdRevision, rfdBcdRevision;
  try {
    bcdRevision = getRevision(_handle);
  } catch (...) {
    bcdRevision = 0;
  }
  try {
    rfdBcdRevision = 0;
    if ((FT_IF_ROBO == _info.ifType) || (FT_IF_RF_DATA == _info.ifType))
     rfdBcdRevision = rfdGetRevision();
  } catch (...) {
    rfdBcdRevision = 0;
  }
  _info.rfdFrequency = 0;
  if ((FT_IF_ROBO == _info.ifType) || (FT_IF_RF_DATA == _info.ifType)) {
    try {
      _info.rfdFrequency = rfdGetFrequency();
    } catch(...) {
      cleanup();
      throw;
    }
  }
  _info.firmwareVer[0] = (bcdRevision >> 24 ) & 0xff;
  _info.firmwareVer[1] = (bcdRevision >> 16 ) & 0xff;
  _info.firmwareVer[2] = (bcdRevision >> 8 ) & 0xff;
  _info.firmwareVer[3] = bcdRevision & 0xff;
  _info.rfdFirmwareVer[0] = (rfdBcdRevision >> 24 ) & 0xff;
  _info.rfdFirmwareVer[1] = (rfdBcdRevision >> 16 ) & 0xff;
  _info.rfdFirmwareVer[2] = (rfdBcdRevision >> 8 ) & 0xff;
  _info.rfdFirmwareVer[3] = rfdBcdRevision & 0xff;
  // revision
  if (FT_IF_ROBO == _info.ifType) {
    _info.filenameFlash1 = getFilenameFlash1();
    _info.filenameFlash2 = getFilenameFlash2();
    _info.filenameRam = getFilenameRam();
  }
  /* we are unable to detect extensions: 
   the following code does not work properly
  // installed modules
    const unsigned char extensions = buffer[15+17];
    _info.slavePresent[0] = ((extensions & EXT_S1)!=0);
    _info.slavePresent[1] = ((extensions & EXT_S2)!=0);
    _info.slavePresent[2] = ((extensions & EXT_S3)!=0);
    _info.radioCommModulePresent = ((extensions & EXT_RADIO_COMM)!=0);
    _info.internetModulePresent  = ((extensions & EXT_INTERNET)!=0);
  */
}

FtInterfaceLibUsb::~FtInterfaceLibUsb() {
  cleanup();
}


void FtInterfaceLibUsb::getIfInfo(IfInfo &info) {
  info = _info;
}


void FtInterfaceLibUsb::writeAndReadAllData(const IfOutputs &outputs, IfInputs
&inputs) throw (XFtComm) {
  assert(ROBO_RFD_IF_COMPLETE_READ_SIZE >= ROBO_IF_COMPLETE_READ_SIZE);
  assert(ROBO_RFD_IF_COMPLETE_WRITE_SIZE >= ROBO_IF_COMPLETE_WRITE_SIZE);
  unsigned char buffer[ROBO_RFD_IF_COMPLETE_WRITE_SIZE + ROBO_RFD_IF_COMPLETE_READ_SIZE];
  memset(buffer, 0, sizeof(buffer));
  int res=0;
  int tries=0;
  switch(_info.ifType) {
    case FT_IF_RF_DATA:
      if (!_rfdConnected) {
      
        memset(&buffer, 0, sizeof(buffer));
        // SUPPORT FOR ROBO RF DATA IS STILL EXPERIMENTIAL!
        // turn radio sender on
        res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
        ROBO_RFD_RF_ON, ROBO_RFD_RF, 1, 
        reinterpret_cast<char*>(buffer), 5, 
        WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  
        if (2 != res) throw 
         XFtComm("FtInterfaceLibUsb: could not activate RF sender");
        if ( (ROBO_RFD_RF_ON+2) != buffer[0] ) throw 
         XFtComm("FtInterfaceLibUsb: could not activate RF sender");
  
        if (0 == buffer[1]) {
          // connection to robo if has been established
          _rfdConnected = true;
        }
        memset(&buffer, 0, sizeof(buffer));
      }
      if (!_rfdConnected) throw XFtCommTimeout(
       "FtInterfaceLibUsb: could not reconnect to robo interface");
      roboIfCopyToBuffer(outputs, buffer);
  



      res = usb_interrupt_write(_handle, ROBO_RFD_IF_OUT_EP, 
       reinterpret_cast<char*>(&(buffer[0])), ROBO_RFD_IF_COMPLETE_WRITE_SIZE, 
       WRITE_TIMEOUT_IN_MS);
      if (res != ROBO_RFD_IF_COMPLETE_WRITE_SIZE) {
        throw XFtComm("FtInterfaceLibUsb: cannot send");
      }
      res = usb_interrupt_read(_handle, ROBO_RFD_IF_IN_EP, 
       reinterpret_cast<char*>(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE])),
       ROBO_RFD_IF_COMPLETE_READ_SIZE, READ_TIMEOUT_IN_MS);
      if (res != ROBO_RFD_IF_COMPLETE_READ_SIZE) {
        throw XFtComm("FtInterfaceLibUsb: cannot receive");
      } 

      inputs = roboIfCopyFromBuffer(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE]));

      // it is rfdata connection, so we don't know if out call succeeded
      // (someone may have turned the robot off after initialisation)
      //
      // TODO check if this has implications on performance!
      memset(buffer, 0, sizeof(buffer));
      res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
       ROBO_RFD_RF_ON, ROBO_RFD_RF, 1, 
       reinterpret_cast<char*>(buffer), 5, 
       WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
      if (2 != res) throw XFtComm("FtInterfaceLibUsb: could not activate RF sender");
      if ( (ROBO_RFD_RF_ON+2) != buffer[0] ) throw 
       XFtComm("FtInterfaceLibUsb: could not activate RF sender");
      if (0 != buffer[1]) {
        _rfdConnected = false;
        throw XFtCommTimeout(
         "FtInterfaceLibUsb: lost connection to robo interface");
      }

      break;

    case FT_IF_ROBO:
      tries = 0;
      do {
        roboIfCopyToBuffer(outputs, buffer);
    
        res = usb_interrupt_write(_handle, ROBO_IF_OUT_EP, 
        reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_WRITE_SIZE, 
        WRITE_TIMEOUT_IN_MS);
        if (res != ROBO_IF_COMPLETE_WRITE_SIZE) {
          res = usb_interrupt_write(_handle, ROBO_IF_OUT_EP, 
          reinterpret_cast<char*>(&(buffer[0])), ROBO_IF_COMPLETE_WRITE_SIZE, 
          WRITE_TIMEOUT_IN_MS);
          if (res != ROBO_IF_COMPLETE_WRITE_SIZE) {
            throw XFtComm("FtInterfaceLibUsb: cannot send");
          }
        }
        res = usb_interrupt_read(_handle, ROBO_IF_IN_EP, 
        reinterpret_cast<char*>(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE])),
        ROBO_IF_COMPLETE_READ_SIZE, READ_TIMEOUT_IN_MS);
        if ((res != ROBO_IF_COMPLETE_READ_SIZE) && (tries>0)) {
          res = usb_interrupt_read(_handle, ROBO_IF_IN_EP, 
          reinterpret_cast<char*>(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE])),
          ROBO_IF_COMPLETE_READ_SIZE, READ_TIMEOUT_IN_MS);
          if ((res != ROBO_IF_COMPLETE_READ_SIZE) && (tries>0)) {
            throw XFtComm("FtInterfaceLibUsb: cannot receive");
          } 
        } 
        ++tries;
      } while (res != ROBO_IF_COMPLETE_READ_SIZE);

      inputs = roboIfCopyFromBuffer(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE]));
      break;

    case FT_IF_IOE:
      roboIoeCopyToBuffer(outputs, buffer);
      res = usb_interrupt_write(_handle, ROBO_IF_OUT_EP, 
       reinterpret_cast<char*>(&(buffer[0])), ROBO_IOE_COMPLETE_WRITE_SIZE, 
       WRITE_TIMEOUT_IN_MS);
      if (res != ROBO_IOE_COMPLETE_WRITE_SIZE) {
        throw XFtComm("FtInterfaceLibUsb: cannot send");
      }
      res = usb_interrupt_read(_handle, ROBO_IF_IN_EP, 
       reinterpret_cast<char*>(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE])),
       ROBO_IOE_COMPLETE_READ_SIZE, READ_TIMEOUT_IN_MS);
      if (res != ROBO_IOE_COMPLETE_READ_SIZE) {
        throw XFtComm("FtInterfaceLibUsb: cannot receive");
      } 
      inputs = roboIoeCopyFromBuffer(&(buffer[ROBO_IF_COMPLETE_WRITE_SIZE]));
      break;

    case FT_IF_SAL:
      throw XFtComm("FtInterfaceLibUsb: Sound+Lights does not support r/w data");

    default:
      assert(0);
  }
}


void FtInterfaceLibUsb::startExecution(ft_program_location location) {
  static const int bufferLength = 1;
  unsigned char buffer[bufferLength];
  // actually we do not know what buffer[0] indicates
  buffer[0] = 0;
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_FLASH_START, location, 0, reinterpret_cast<char*>(buffer), 
   bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  
  if (bufferLength != res) throw XFtComm("cannot start program");
  if (0x01 == buffer[0]) return;
  if (0xFB == buffer[0]) throw XFtComm("program crc error");
  if (0xF4 == buffer[0]) throw XFtComm("no program"); // official docs (serial port)
  if (0xE8 == buffer[0]) throw XFtComm("no program"); // measured
  // unecpected restoponse
  std::cerr << "b[0]: " << std::hex << (int)buffer[0] << std::endl;
  assert(0);
}


void FtInterfaceLibUsb::stopExecution() { 
  static const int bufferLength = 1;
  unsigned char buffer[bufferLength];
  // actually we do not know what buffer[0] indicates
  buffer[0] = 1;
  int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
   ROBO_IF_FLASH_STOP, 0, 0, reinterpret_cast<char*>(buffer), 
   bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  
  if (bufferLength != res) throw XFtComm("cannot stop program");
  if (0x01 == buffer[0]) return;
  if (0xEC == buffer[0]) throw XFtComm("program not active");
  // unecpected restoponse
  std::cerr << "b[0]: " << std::hex << (int)buffer[0] << std::endl;
  assert(0);
}


bool FtInterfaceLibUsb::isExecutionActive(ft_program_location *plocation) {
  if (FT_IF_ROBO == _info.ifType) {
    const int packetsize = 3;
    unsigned char buffer[packetsize];
  
    int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
    ROBO_IF_INTERN, ROBO_IF_INTERN_MODE, 0, reinterpret_cast<char*>(buffer), 
    packetsize, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
  
    const unsigned char c = ~ROBO_IF_INTERN_MODE;
    if ((packetsize != res) || (c != buffer[0])) {
      cleanup();
      throw XFtComm("cannot communincate");
    }
    assert(buffer[1] <= 1); // 0: online mode; 1: program active
    assert(buffer[2] <= 2); // location
    
    if (1 == buffer[1]) {
      if (0 != plocation) {
        *plocation = static_cast<ft_program_location>(buffer[2]);
      }
      return true;
    }
  }
  return false;
}



size_t FtInterfaceLibUsb::writeAndReadAllData(unsigned char* outBuf, size_t outSize, unsigned char* inBuf, size_t maxSize, bool verbose) {
  int res = usb_bulk_write(_handle, USB_ENDPOINT_OUT + 1, (char*)outBuf, outSize, WRITE_TIMEOUT_IN_MS);
  if (verbose) {
    std::cout << "SAL? bulk write: wrote " << res << " bytes" << std::endl;
      for (int j=0; j<res; j++) std::cout << (int)inBuf[j] << " "; 
      if (res>0) std::cout << std::endl;
  }
  if (res<0) throw XFtComm("write error");
  memset(inBuf, 0, maxSize);
  int tries = 40;
  do {
    res = usb_bulk_read(_handle, USB_ENDPOINT_IN + 1, 
    reinterpret_cast<char*>(inBuf), 
    maxSize, READ_TIMEOUT_IN_MS*10);
    if (verbose) {
      std::cout << "SAL? bulk read, received " << res << " bytes:" << std::endl;   
      for (int j=0; j<res; j++) std::cout << (int)inBuf[j] << " "; 
      if (res>0) std::cout << std::endl;
    }
    --tries;
  } while ( (res<=0) && (tries) );
  if (!tries) throw XFtComm("read error");
  return res;
}


void FtInterfaceLibUsb::storeWavFiles(std::string fn1, std::string fn2, std::string fn3) {

  if (FT_IF_SAL == _info.ifType) {
  
    SalWaveSource src1(fn1);
    SalWaveSource src2(fn2);
    SalWaveSource src3(fn3);
    
    std::cout << std::dec;
    std::cout << "\"" << fn1 << "\": " << src1.bytesAvail() << " bytes" << std::endl;
    std::cout << "\"" << fn2 << "\": " << src2.bytesAvail() << " bytes" << std::endl;
    std::cout << "\"" << fn3 << "\": " << src3.bytesAvail() << " bytes" << std::endl;
  
    // msg
    char msgFn[0x118];
    memset(msgFn, 0, sizeof(msgFn));
    msgFn[0] = 0xf1;
    msgFn[2] = 0x01;
    msgFn[4] = 0x17;
    // 168+80=248
    msgFn[256+ 8] = 0xbc;
    //msgFn[256+ 9] = 0x??;
    //msgFn[256+10] = 0x??;
    //msgFn[256+11] = 0x??;
    //msgFn[256+12] = 0x??;
    //msgFn[256+16] = 0x??;
    //msgFn[256+17] = 0x??;
    msgFn[256+20] = 0xe0;
    msgFn[256+21] = 0x27;
    msgFn[256+22] = 0x4c;
    assert(256+23 == 280-1);
    
    for (int i=0; i<280; ++i) {
      unsigned char c = msgFn[i];
      if (i%16==0) std::cout << std::endl;
      std::cout << std::setw(2) << std::hex << (unsigned int)c << " ";
    }
    unsigned char buf[500];
    
    
    
    unsigned char msgPos[0x118] = {
      0xf0, 0x00, 0x17, 0x07, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x20, 0x00, 0x00, 0x07, 0x20, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char msg7[0x118] = {
      0xf1, 0x00, 0x02, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x20, 0x00, 0x02, 0x09, 0x20, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  

    
    
    
    
    
    
    
    for (int file=0; file<3; ++file) {
    
      // prepare data
      switch (file+10) {
        case 0: strncpy(&(msgFn[  8]), fn1.c_str(), 80);  // FIXME: maybe 79 is big enough
          break;
        case 1: strncpy(&msgFn[ 88], fn2.c_str(), 80);
          break;
        case 2: strncpy(&msgFn[168], fn3.c_str(), 80);
          break;
      }
            
      unsigned int spaceleft=0x0111;
      unsigned int plen=0x0123, badr=3+0x125*file, blen=plen*256;
            
      msgPos[4] = (spaceleft>>8)&(0xff);
      msgPos[5] = spaceleft&0xff;
      
      size_t base = 29+file*20;
      msgPos[base] = file;
      msgPos[base+1+0] = (plen>>8)&0xff;
      msgPos[base+1+1] = plen&0xff;
        
      msgPos[base+3+0] = (badr>>8)&0xff;
      msgPos[base+3+1] = badr&0xff;
        
      msgPos[base+5+0] = (blen>>24)&0xff;
      msgPos[base+5+1] = (blen>>16)&0xff;
      msgPos[base+5+2] = (blen>>8)&0xff;
      msgPos[base+5+3] = blen&0xff;

      writeAndReadAllData(msgPos, sizeof(msgPos), buf, sizeof(buf), 1);
std::cout << std::endl << "fn" << std::endl << std::endl;            
      writeAndReadAllData((unsigned char*)msgFn, sizeof(msgFn), buf, sizeof(buf), 1);
      writeAndReadAllData(msg7, sizeof(msg7), buf, sizeof(buf), 1);
      
    }
    
  } else {
    throw XFtComm("write flash not supported by device");
  }
}

void FtInterfaceLibUsb::deleteFlash() {
  if (FT_IF_SAL == _info.ifType) {
  
#ifndef NDEBUG    
    std::cout << "==========================DELETING SAL DATA=================================" << std::endl;
    std::cout << "==========================DELETING SAL DATA=================================" << std::endl;
#endif
    unsigned char msg1[16] = {0xbc,1,0,0, 0,4,6,7, 8,9,0xa,0xb, 0xc,0xd,0xe,4};
    unsigned char msg2[16] = {0xbc,1,0,0, 0,5,6,7, 8,9,0xa,0xb, 0xc,0xd,0xe,5};
    unsigned char msg3[16] = {0xbc,1,0,0, 0,6,6,7, 8,9,0xa,0xb, 0xc,0xd,0xe,6};
    unsigned char msg4[16] = {0xbc,0x30,0,0, 0,0,7,0xf8, 1,0,0,0, 0,0,0,0};
    unsigned char msg5[0x118] = {
      0xf0, 0x00, 0x17, 0x07, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x20, 0x00, 0x00, 0x07, 0x20, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char msg6[0x118] = {
      0xf1, 0x00, 0x01, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x20, 0x00, 0x01, 0x08, 0x20, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char msg7[0x118] = {
      0xf1, 0x00, 0x02, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x20, 0x00, 0x02, 0x09, 0x20, 0x20, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  
    
    
    unsigned char buf[500];
    try {
      usb_clear_halt(_handle, 1);
#ifdef NDEBUG
      static const bool verbose = false;
#else
      static const bool verbose = true;      
#endif
      writeAndReadAllData(msg1, sizeof(msg1), buf, sizeof(buf), verbose);
      writeAndReadAllData(msg2, sizeof(msg2), buf, sizeof(buf), verbose);
      writeAndReadAllData(msg3, sizeof(msg3), buf, sizeof(buf), verbose);
      { 
        int adr = 0;
        int cnt = 7;
        for (int i=0; i<=255; ++i) {
          msg4[2]=(adr>>8)&0xff;
          msg4[3]=adr&0xff;
          msg4[4]=0xff-msg4[2];
          msg4[5]=0xff-msg4[3];
          msg4[6]=cnt;
          msg4[7]=0xff-cnt;
          writeAndReadAllData(msg4, sizeof(msg4), buf, sizeof(buf));
          cnt++;
          adr+=8;
        }
        assert(adr==0x0800);
      }
      writeAndReadAllData(msg5, sizeof(msg5), buf, sizeof(buf), verbose);
      writeAndReadAllData(msg6, sizeof(msg6), buf, sizeof(buf), verbose);
      writeAndReadAllData(msg7, sizeof(msg7), buf, sizeof(buf), verbose);
    
    } catch (XFtComm &) {
      throw XFtComm("could not read from Sound+Lights module");
    }
    
    
    
    
  } else {
    throw XFtComm("deleteFlash not supported by device");
  }
}

/*
standard:

==========================DELETING SAL DATA=================================
==========================DELETING SAL DATA=================================
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 0 76 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 -83 0 3 0 2 -84 16 0 0 0 0 0 0 0 0 0 0 0 1 2 -78 2 -80 0 2 -79 32 0 0 0 0 0 0 0 0 0 0 0 2 2 82 5 98 0 2 81 86 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 4 0 0 0 0 0 0 0 0 0 0 0 
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 0 76 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 -83 0 3 0 2 -84 16 0 0 0 0 0 0 0 0 0 0 0 1 2 -78 2 -80 0 2 -79 32 0 0 0 0 0 0 0 0 0 0 0 2 2 82 5 98 0 2 81 86 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 5 0 0 0 0 0 0 0 0 0 0 0 
SAL bulk read, received 0 bytes:
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 0 76 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 -83 0 3 0 2 -84 16 0 0 0 0 0 0 0 0 0 0 0 1 2 -78 2 -80 0 2 -79 32 0 0 0 0 0 0 0 0 0 0 0 2 2 82 5 98 0 2 81 86 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 6 0 0 0 0 0 0 0 0 0 0 0 
SAL bulk read, received 0 bytes:


empty:
==========================DELETING SAL DATA=================================
==========================DELETING SAL DATA=================================
SAL bulk read, received 0 bytes:
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 7 -3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 4 0 0 0 0 0 0 0 0 0 0 0 
SAL bulk read, received 0 bytes:
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 7 -3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 5 0 0 0 0 0 0 0 0 0 0 0 
SAL bulk read, received 0 bytes:
SAL bulk read, received 0 bytes:
SAL bulk read, received 280 bytes:
-16 0 23 7 -3 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -68 1 0 0 6 0 0 0 0 0 0 0 0 0 0 0 
*/





/////////////////////////////////////////////////////////////////////////////////////////////////
//
// PROGRAM DOWNLOAD TEST
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
//    {
//     static const int bufferLength = 3;
//     unsigned char buffer[bufferLength];
//     // actually we do not know what buffer[0] indicates
//     buffer[0] = 0;
//     int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
//     0xF0, 0x3, 0, reinterpret_cast<char*>(buffer), 
//     bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
//     
//     if (bufferLength != res) throw XFtComm("fail status");
//     // unecpected restoponse
//     std::cerr << "0xF0, 0x3, 0x0: b[0..2]: " << std::hex << (int)buffer[0] << " " << 
//                                                 std::hex << (int)buffer[1] << " " <<
//                                                 std::hex << (int)buffer[2] << std::endl;
//   }

  static const int dest = 0x101; // flash2
  {
    static const int bufferLength = 1;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    buffer[0] = 0;
    int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
    0xF0, 0x20, 0, reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail1");
    // unecpected restoponse
    std::cerr << "0xF0, 0x20, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  {
    static const int bufferLength = 0x50;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    for (int i=0; i<bufferLength; ++i) buffer[i] = 0;
    buffer[0] = 't';
    buffer[1] = 's';
    buffer[2] = 't';
    int res = usb_control_msg(_handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR, 
    0x10, dest, 3, reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail2");
    // unecpected restoponse
    std::cerr << "0x10, 0x10?, 0x2: b: " << buffer << std::endl;
    
  }
  for (int i=0; i<10; ++i) {
    std::cout << "iteration " << i << std::endl;
    try {
      static const int bufferLength = 1;
      unsigned char buffer[bufferLength];
      // actually we do not know what buffer[0] indicates
      buffer[0] = 0;
      int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
      0x20, 0, 0, reinterpret_cast<char*>(buffer), 
      bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
      
      if (bufferLength != res) throw XFtComm("fail3");
      // unecpected restoponse
      std::cerr << "0x20, 0x0, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
      
    } catch (std::exception &x) {
      std::cout << x.what() << std::endl;
    }
        
  }
  {// 1. block
    static const int bufferLength = 128;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    for (int i=0; i<bufferLength; ++i) buffer[i] = 0;
    buffer[1] = 1;
    
    int res = usb_control_msg(_handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR, 
    0x11, 0x1, crc16(bufferLength, buffer) , reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail4");
    // unecpected restoponse
    //std::cerr << "0xF0, 0x20, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  {
    static const int bufferLength = 1;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    buffer[0] = 0;
    int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
    0x20, 0, 0, reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail5");
    // unecpected restoponse
    std::cerr << "0x20, 0x0, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  {// 2. block
    static const int bufferLength = 128;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    for (int i=0; i<bufferLength; ++i) buffer[i] = 0;
    buffer[1] = 2;
    int res = usb_control_msg(_handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR, 
    0x11, 0x2, crc16(bufferLength, buffer), reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail4b");
    // unecpected restoponse
    //std::cerr << "0xF0, 0x20, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  {
    static const int bufferLength = 1;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    buffer[0] = 0;
    int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
    0x20, 0, 0, reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail5");
    // unecpected restoponse
    std::cerr << "0x20, 0x0, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  { // 3. block
    static const int bufferLength = 128;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    for (int i=0; i<bufferLength; ++i) buffer[i] = 0;
    buffer[1] = 2;
    int res = usb_control_msg(_handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR, 
    0x11, 0x3, crc16(bufferLength, buffer), reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail4b");
    // unecpected restoponse
    //std::cerr << "0xF0, 0x20, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }
  {
    static const int bufferLength = 1;
    unsigned char buffer[bufferLength];
    // actually we do not know what buffer[0] indicates
    buffer[0] = 0;
    int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
    0x20, 0, 0, reinterpret_cast<char*>(buffer), 
    bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
    
    if (bufferLength != res) throw XFtComm("fail5");
    // unecpected restoponse
    std::cerr << "0x20, 0x0, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
    
  }  
//   {
//     static const int bufferLength = 0xe;
//     unsigned char buffer[bufferLength];
//     // actually we do not know what buffer[0] indicates
//     buffer[0] = 0;
//     int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
//     0xF0, 2, 0, reinterpret_cast<char*>(buffer), 
//     bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
//     
//     if (bufferLength != res) throw XFtComm("fail6");
//     // unecpected restoponse
//     std::cerr << "0xF0, 0x2, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
//     
//   }
//   {
//     static const int bufferLength = 0x5;
//     unsigned char buffer[bufferLength];
//     // actually we do not know what buffer[0] indicates
//     buffer[0] = 0;
//     int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
//     0xF0, 1, 0, reinterpret_cast<char*>(buffer), 
//     bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
//     
//     if (bufferLength != res) throw XFtComm("fail6b");
//     // unecpected restoponse
//     std::cerr << "0xF0, 0x1, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
//     
//   }
//   {
//     static const int bufferLength = 0x20;
//     unsigned char buffer[bufferLength];
//     // actually we do not know what buffer[0] indicates
//     buffer[0] = 0;
//     int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
//     0xF0, 0x11, 0, reinterpret_cast<char*>(buffer), 
//     bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
//     
//     if (bufferLength != res) throw XFtComm("fail6");
//     // unecpected restoponse
//     std::cerr << "0xF0, 0x11, 0x0: b[0]: " << std::hex << (int)buffer[0] << std::endl;
//   }
//   
//    {
//     static const int bufferLength = 3;
//     unsigned char buffer[bufferLength];
//     // actually we do not know what buffer[0] indicates
//     buffer[0] = 0;
//     int res = usb_control_msg(_handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR, 
//     0xF0, 0x3, 0, reinterpret_cast<char*>(buffer), 
//     bufferLength, WRITE_TIMEOUT_IN_MS + READ_TIMEOUT_IN_MS);
//     
//     if (bufferLength != res) throw XFtComm("fail status");
//     // unecpected restoponse
//     std::cerr << "0xF0, 0x3, 0x0: b[0..2]: " << std::hex << (int)buffer[0] << " " << 
//                                                 std::hex << (int)buffer[1] << " " <<
//                                                 std::hex << (int)buffer[2] << std::endl;
//   }



  std::cout << "fn: " << getFilenameFlashHelper(dest - 0x100) << std::endl;
#endif


} // namespace
#endif
// EOF
