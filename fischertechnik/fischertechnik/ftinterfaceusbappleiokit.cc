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
#ifdef HAVE_APPLEIOKIT
#include "ftinterfaceusbappleiokit.h"
#include <fcntl.h>   /* File control definitions */
#include <iostream>
#include <sstream>
#include <errno.h>   /* EAGAIN, etc. */
#include <cassert>

/** @file
implementation */
namespace ftapi {

void FtInterfaceUsbAppleIOKit::initVar() {
  m_readBuffer = NULL;
  m_writeBuffer = NULL;
  m_usbInterfaceInterface = NULL;
  m_masterPortAlloced = false;
}

void FtInterfaceUsbAppleIOKit::cleanup() {
  // Close the interface.
  if (NULL != m_usbInterfaceInterface) {
    if (NULL != (*m_usbInterfaceInterface)) {
      (*m_usbInterfaceInterface)->USBInterfaceClose(m_usbInterfaceInterface);
      (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
  
      // Close the device.
      (*m_usbDeviceInterface)->USBDeviceClose(m_usbDeviceInterface);
      (*m_usbDeviceInterface)->Release(m_usbDeviceInterface);
    }
  }
  
  // Terminate the IOKit access.
  if (m_masterPortAlloced) {
    mach_port_deallocate(mach_task_self(), m_masterPort);
    m_masterPortAlloced = false;
  }
  
  // Deallocate buffers.
  delete[] m_readBuffer;
  m_readBuffer = NULL;
  delete[] m_writeBuffer;
  m_writeBuffer = NULL;
}

FtInterfaceUsbAppleIOKit::FtInterfaceUsbAppleIOKit(std::string device) throw(XFtComm)
{
  // init members, after that calling cleanup() is allowed
  initVar();

  // Allocate buffers.
  m_readBuffer = new unsigned char[FTRoboInterfaceUsbIOKitReadSize];
  m_writeBuffer = new unsigned char[FTRoboInterfaceUsbIOKitWriteSize];
  
  /// Establish access to the IOKit.
  /// Note that we cannot use constants (really?)
  /// for our USB product/vendor ids.
  int s_vendorID = FTRoboInterfaceUsbVendorID;
  int s_productID = FTRoboInterfaceUsbProductID;
  
  // first create a master_port for my task
  kern_return_t kr = IOMasterPort(MACH_PORT_NULL, &m_masterPort);
  if (0 != kr)
  {
    cleanup();
    throw XFtComm("Couldn't create a master IOKit Port.");
  }
  m_masterPortAlloced = true;

  // Match services of class IOUSBDevice and subclasses
  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOUSBDeviceClassName);	
  if (NULL == matchingDict)
  {
    cleanup();
    throw XFtComm("Can't create a IOKit USB matching dictionary.");
  }
  
  // Add our vendor and product IDs to the matching criteria
  CFDictionarySetValue( matchingDict, CFSTR(kUSBVendorID), 
                        CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &s_vendorID)); 
  CFDictionarySetValue( matchingDict, CFSTR(kUSBProductID), 
                        CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &s_productID)); 
  
  // Try to find a IOKit service that handles our Fischertechnik device.
  // First, obtain a serviceIterator to access the matching services.
  // Maybe, there is a service for each device attached to the USB bus.
  // However, this has to be clarified using the IOKit docs.
  io_iterator_t serviceIterator;
  kr = IOServiceGetMatchingServices( kIOMasterPortDefault, matchingDict, &serviceIterator );
  if(kIOReturnSuccess != kr)
  {
    cleanup();
    throw XFtComm("Can't get a IOKit USB matching service.");
  }
  
  // Use the obtained service iterator to access the first service available,
  // create a plugin interface (I must not cease trying to understand this terminology!), 
  // then obtain a device interface to proceed further.
  IOCFPlugInInterface **plugInInterface = NULL;
  m_usbDeviceInterface = NULL;
  io_service_t usbDeviceService; // Our usb device service we obtain through the iterator.
  SInt32       score; // <----+----- What is the purpose of these two fellows?
  HRESULT      res;   // <---/
  
  // Obtain (probably) the first attached device; see above.
  usbDeviceService = IOIteratorNext(serviceIterator);
  
  // We need the USB device interface. In order to access it, we have to ask
  // the plugin interface of the device service for it.
  // As soon as we have the device interface, we may discard all references
  // to the device service and the plugin interface.
  // Actually, what is a plugin Interface? <--- Needs to be clarified.
  kr = IOCreatePlugInInterfaceForService(usbDeviceService, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
  if ((kIOReturnSuccess != kr) || !plugInInterface)
  {
    // TODO: close the IOKit.
    throw XFtComm("Unable to create a plugin.");
  }
  res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID), (LPVOID*)&m_usbDeviceInterface);
  if (res || !m_usbDeviceInterface) // <--- TODO: clean this check up.
  {
    // TODO: close the IOKit.
    throw XFtComm("Couldn't create a device interface.n");
  }
  // Discard plugin interface and service.
  IODestroyPlugInInterface(plugInInterface);
  IOObjectRelease(usbDeviceService);
    
  // Open the device to access its interfaces.
  // We need to communicate with the first (and only) interface of the device.
  // First, we have to request a service handling the interface. Then, we ask
  // the plugin interface of this service for the interface interface which
  // we may use to communicate of the requested interface of the device.
  kr = (*m_usbDeviceInterface)->USBDeviceOpen(m_usbDeviceInterface);
  if (kIOReturnSuccess != kr)
  {
    // TODO: close the IOKit.
    (*m_usbDeviceInterface)->Release(m_usbDeviceInterface);
    throw XFtComm("unable to open device.");
  }
  
   
  // Create a request to find our desired interface.
  IOUSBFindInterfaceRequest	request;
  request.bInterfaceClass = kIOUSBFindInterfaceDontCare;
  request.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
  request.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
  request.bAlternateSetting = kIOUSBFindInterfaceDontCare;
  
  // Request the first matching USB interface service through an iterator.
  io_iterator_t		interfaceIterator;
  io_service_t		usbInterfaceService;
  kr = (*m_usbDeviceInterface)->CreateInterfaceIterator(m_usbDeviceInterface, &request, &interfaceIterator);
  usbInterfaceService = IOIteratorNext(interfaceIterator);
  
  // Ask the plugin interface of the device service for the interface-interface.
  // As soon as we have the interface interface, we may discard all references
  // to the device service and the plugin interface.
  kr = IOCreatePlugInInterfaceForService(usbInterfaceService, kIOUSBInterfaceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score);
  if ((kIOReturnSuccess != kr) || (NULL == plugInInterface))
  {
    // TODO: release IOKit.
    throw XFtComm("Unable to create a plugin interface.");
  }
  res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID), (LPVOID*) &m_usbInterfaceInterface);
  if (res || !m_usbInterfaceInterface)
  {
    // TODO: release IOKit.
    throw XFtComm("Couldn't create an IOUSBInterfaceInterface.");
  }
  // Discard plugin interface and service.
  IODestroyPlugInInterface(plugInInterface);
  IOObjectRelease(usbInterfaceService);
  
  // Open the interface interface to have a pipe instantiated for each
  // endpoint of the interface. Thus, we can access all endpoints of
  // the device afterwards.
  // IMPORTANT: When accessing the endpoints, we rely entirely on the
  // order in which the IOKit sorts them. That is, we access them using
  // indices as opposed to hardwired endpoint addresses. It would be more wise
  // to iterate over all available endpoints and determine which of them
  // are our interrupt-out- and interrupt-in-endpoint respectively.
  kr = (*m_usbInterfaceInterface)->USBInterfaceOpen(m_usbInterfaceInterface);
  if (kIOReturnSuccess != kr)
  {
    // TODO: release IOKit.
    (void) (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
    throw XFtComm("Unable to open interface.");
  }
   
  // Set the first character to be transmitted to the required magic number
  // which makes the ROBO INTERFACE behave.
  m_writeBuffer[0] = ROBO_IF_COMPLETE;
   
  // Write to the device for the first time to make sure it behaves properly.
  kr = (*m_usbInterfaceInterface)->WritePipe(m_usbInterfaceInterface,
                                             FTRoboInterfaceWriteEndpoint,
                                             m_writeBuffer,
                                             FTRoboInterfaceUsbIOKitWriteSize);
  if (kIOReturnSuccess != kr)
  {
    // TODO: close device, release IOKit.
    (void) (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
    throw XFtComm("Unable to write to endpoint.");
  }
  
  // This will be changed to both request the number of bytes to read and to reflect the number of bytes successfully read.
  UInt32 numBytesRead = FTRoboInterfaceUsbIOKitReadSize; 
  kr = (*m_usbInterfaceInterface)->ReadPipe(m_usbInterfaceInterface,
                                            FTRoboInterfaceReadEndpoint,
                                            m_readBuffer,
                                            &numBytesRead);
  if (kIOReturnSuccess != kr)
  {
    // TODO: close device, release IOKit.
    (void) (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
    throw XFtComm("Unable to read from endpoint.");
  }

  /// Aquire serial number and firmware revision.
  
  // Now we have successfully established communications with our USB fischertechnik RoboInterface.
  
  // Request the device descriptor.
  IOUSBDeviceDescriptor deviceDescriptorBuffer;
  IOUSBDevRequest deviceDescriptorRequest;
  deviceDescriptorRequest.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
  deviceDescriptorRequest.bRequest = kUSBRqGetDescriptor;
  deviceDescriptorRequest.wValue = kUSBDeviceDesc << 8;
  deviceDescriptorRequest.wIndex = 0;
  // TODO: HF thinks, wLength should be 0 (not data to transfer _to_ the interf.)
  deviceDescriptorRequest.wLength = sizeof(IOUSBDeviceDescriptor);
  deviceDescriptorRequest.pData = &deviceDescriptorBuffer;
  kr = (*m_usbDeviceInterface)->DeviceRequest(m_usbDeviceInterface, &deviceDescriptorRequest);
  if(kIOReturnSuccess != kr)
  {
    // TODO: release IOKit and just everything.
    throw XFtComm("Reading the device descriptor failed.");
  }

  // Request the languages string descriptor.
  char langStringBuffer[256];
  IOUSBDevRequest langRequest;
  langRequest.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
  langRequest.bRequest = kUSBRqGetDescriptor;
  langRequest.wValue = (kUSBStringDesc << 8) + 0; //deviceDescriptorBuffer.iSerialNumber;
  langRequest.wIndex = 0x0000;
  langRequest.wLength = 255;
  langRequest.pData = langStringBuffer;
  kr = (*m_usbDeviceInterface)->DeviceRequest(m_usbDeviceInterface, &langRequest);
  if(kIOReturnSuccess != kr)
  {
    // TODO: release IOKit and just everything.
    throw XFtComm("Reading the language string descriptor failed.");
  }
  
  // Compose the language id of the two corresponding bytes in the languages string descriptor.
  int langid = langStringBuffer[2] | (langStringBuffer[3] << 8);
  
  // Request the serial number string descriptor.
  char serialStringBuffer[256];
  IOUSBDevRequest serialRequest;
  serialRequest.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
  serialRequest.bRequest = kUSBRqGetDescriptor;
  serialRequest.wValue = (kUSBStringDesc << 8) + deviceDescriptorBuffer.iSerialNumber;
  serialRequest.wIndex = langid;
  serialRequest.wLength = 255;
  serialRequest.pData = serialStringBuffer;
  kr = (*m_usbDeviceInterface)->DeviceRequest(m_usbDeviceInterface, &serialRequest);
  if(kIOReturnSuccess != kr)
  {
    // TODO: release IOKit and just everything.
    throw XFtComm("Reading the serial number failed.");
  }
  
  // Convert serial number string from unicode.
  char serialASCIIBuffer[9];
  for (unsigned int i=2; (( (i/2-1) < 9) && (i <= serialRequest.wLenDone - 2)); i += 2)
    serialASCIIBuffer[i/2-1] = serialStringBuffer[i];
  serialASCIIBuffer[8] = '\0';

  // Convert serial number string into integer.
  std::istringstream serialStream(std::string("0x") + std::string(serialASCIIBuffer)); 
  int serialNo;
  serialStream >> std::hex >> serialNo;
  if (!serialStream.eof()) serialNo = 0; // string could not be parsed completely
  
  // Request the firmware number string descriptor.
  static const char iRev = 4; /* magic number, hope it does not change in the future */
  char firmwareStringBuffer[256];
  IOUSBDevRequest firmwareRequest;
  firmwareRequest.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBStandard, kUSBDevice);
  firmwareRequest.bRequest = kUSBRqGetDescriptor;
  firmwareRequest.wValue = (kUSBStringDesc << 8) + iRev;
  firmwareRequest.wIndex = langid;
  firmwareRequest.wLength = 255;
  firmwareRequest.pData = firmwareStringBuffer;
  kr = (*m_usbDeviceInterface)->DeviceRequest(m_usbDeviceInterface, &firmwareRequest);
  if(kIOReturnSuccess != kr)
  {
    cleanup();
    throw XFtComm("Reading the firmware revision failed.");
  }
  
  // Convert firmware revision string from unicode.
  int fwLen = (firmwareRequest.wLenDone-2)/2; // length of ASCII firmware revision string after conversion.
  fwLen = std::min(127, fwLen); // buffer size
  char firmwareASCIIBuffer[128];
  for (int i=2; i <= fwLen*2; i += 2)
    firmwareASCIIBuffer[i/2-1] = firmwareStringBuffer[i];
  firmwareASCIIBuffer[fwLen] = '\0';
  
  // Parse the firmware revision string.
  int rev = 0;
  int revminor = 0;
  int ptr = 0;
  while (ptr < fwLen)
  {
    if (0 == firmwareASCIIBuffer[ptr])
    {
      rev = (rev << 8) | (revminor & 0xFF);
      /* finished */
      ptr = fwLen;
    }
    else if ('.' == firmwareASCIIBuffer[ptr])
    {
      rev = (rev << 8) | (revminor & 0xFF);
      revminor = 0;
    }
    else if ((firmwareASCIIBuffer[ptr] < 48) || (firmwareASCIIBuffer[ptr] > 57))
    {
      /* parse error */
      ptr = fwLen;
      rev = 0;
    }
    else
    {
      revminor *= 10;
      revminor += firmwareASCIIBuffer[ptr] - 48;
    }
    ++ptr;
  }
  int bcdRevision = rev;

  /// Set device information based on gathered pieces of information.
  _info.ifType = FT_IF_ROBO;
  _info.ifConn = FT_IC_USB_IOKIT;
  _info.device = device;
  _info.serialNo = serialNo;
  _info.firmwareVer[0] = (bcdRevision >> 24 ) & 0xff;
  _info.firmwareVer[1] = (bcdRevision >> 16 ) & 0xff;
  _info.firmwareVer[2] = (bcdRevision >> 8 ) & 0xff;
  _info.firmwareVer[3] = bcdRevision & 0xff;

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

FtInterfaceUsbAppleIOKit::~FtInterfaceUsbAppleIOKit() {
  cleanup();
}

void FtInterfaceUsbAppleIOKit::getIfInfo(IfInfo &info) {
  info = _info;
}

void FtInterfaceUsbAppleIOKit::writeAndReadAllData(const IfOutputs &outputs, IfInputs &inputs) throw (XFtComm) {
  m_writeBuffer[ 0]=ROBO_IF_COMPLETE;
  m_writeBuffer[ 1]=outputs.data[FT_MASTER];
  m_writeBuffer[ 2]=(((outputs.speed[FT_MASTER][0])&7)   )
    +(((outputs.speed[FT_MASTER][1])&7)<<3)
    +(((outputs.speed[FT_MASTER][2])&3)<<6);
  m_writeBuffer[ 3]=(((outputs.speed[FT_MASTER][2])&4)>>2)
    +(((outputs.speed[FT_MASTER][3])&7)<<1)
    +(((outputs.speed[FT_MASTER][4])&7)<<4)
    +(((outputs.speed[FT_MASTER][5])&1)<<7);
  m_writeBuffer[ 4]=(((outputs.speed[FT_MASTER][5])&6)>>1)
    +(((outputs.speed[FT_MASTER][6])&7)<<2)
    +(((outputs.speed[FT_MASTER][7])&7)<<5);
  m_writeBuffer[ 5]=outputs.data[FT_SLAVE1];
  m_writeBuffer[ 6]=(((outputs.speed[FT_SLAVE1][0])&7)   )
    +(((outputs.speed[FT_SLAVE1][1])&7)<<3)
    +(((outputs.speed[FT_SLAVE1][2])&3)<<6);
  m_writeBuffer[ 7]=(((outputs.speed[FT_SLAVE1][2])&4)>>2)
    +(((outputs.speed[FT_SLAVE1][3])&7)<<1)
    +(((outputs.speed[FT_SLAVE1][4])&7)<<4)
    +(((outputs.speed[FT_SLAVE1][5])&1)<<7);
  m_writeBuffer[ 8]=(((outputs.speed[FT_SLAVE1][5])&6)>>1)
    +(((outputs.speed[FT_SLAVE1][6])&7)<<2)
    +(((outputs.speed[FT_SLAVE1][7])&7)<<5);
  m_writeBuffer[ 9]=outputs.data[FT_SLAVE2];
  m_writeBuffer[10]=(((outputs.speed[FT_SLAVE2][0])&7)   )
    +(((outputs.speed[FT_SLAVE2][1])&7)<<3)
    +(((outputs.speed[FT_SLAVE2][2])&3)<<6);
  m_writeBuffer[11]=(((outputs.speed[FT_SLAVE2][2])&4)>>2)
    +(((outputs.speed[FT_SLAVE2][3])&7)<<1)
    +(((outputs.speed[FT_SLAVE2][4])&7)<<4)
    +(((outputs.speed[FT_SLAVE2][5])&1)<<7);
  m_writeBuffer[12]=(((outputs.speed[FT_SLAVE2][5])&6)>>1)
    +(((outputs.speed[FT_SLAVE2][6])&7)<<2)
    +(((outputs.speed[FT_SLAVE2][7])&7)<<5);
  m_writeBuffer[13]=outputs.data[FT_SLAVE3];
  m_writeBuffer[14]=(((outputs.speed[FT_SLAVE3][0])&7)   )
    +(((outputs.speed[FT_SLAVE3][1])&7)<<3)
    +(((outputs.speed[FT_SLAVE3][2])&3)<<6);
  m_writeBuffer[15]=(((outputs.speed[FT_SLAVE3][2])&4)>>2)
    +(((outputs.speed[FT_SLAVE3][3])&7)<<1)
    +(((outputs.speed[FT_SLAVE3][4])&7)<<4)
    +(((outputs.speed[FT_SLAVE3][5])&1)<<7);
  m_writeBuffer[16]=(((outputs.speed[FT_SLAVE3][5])&6)>>1)
    +(((outputs.speed[FT_SLAVE3][6])&7)<<2)
    +(((outputs.speed[FT_SLAVE3][7])&7)<<5);
  
  /* buffers prepared, write and then read using the IOKit here */
  kern_return_t kr = 0;
  UInt32 readSize;
  kr = (*m_usbInterfaceInterface)->WritePipe(m_usbInterfaceInterface,
                                             FTRoboInterfaceWriteEndpoint,
                                             m_writeBuffer,
                                             FTRoboInterfaceUsbIOKitWriteSize);
  if (kIOReturnSuccess != kr)
  {
    // TODO: close device, release IOKit.
    (void) (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
    throw XFtComm("Unable to write to endpoint.");
  }
  
  readSize = FTRoboInterfaceUsbIOKitReadSize;
  kr = (*m_usbInterfaceInterface)->ReadPipe(m_usbInterfaceInterface,
                                            FTRoboInterfaceReadEndpoint,
                                            m_readBuffer,
                                            &readSize);
  if (kIOReturnSuccess != kr)
  {
    // TODO: close device, release IOKit.
    (void) (*m_usbInterfaceInterface)->Release(m_usbInterfaceInterface);
    throw XFtComm("Unable to read from endpoint.");
  }
  /* done writing and reading, interpret the buffer now */
  
  inputs.data[FT_MASTER] = m_readBuffer[ 0];
  inputs.data[FT_SLAVE1] = m_readBuffer[ 1];
  inputs.data[FT_SLAVE2] = m_readBuffer[ 2];
  inputs.data[FT_SLAVE3] = m_readBuffer[ 3];
  inputs.ax[FT_MASTER]   = m_readBuffer[ 4] + (static_cast<int>((m_readBuffer[ 8]   )&3)<<8);
  inputs.ay              = m_readBuffer[ 5] + (static_cast<int>((m_readBuffer[ 8]>>2)&3)<<8);
  inputs.a1              = m_readBuffer[ 6] + (static_cast<int>((m_readBuffer[ 8]>>4)&3)<<8);
  inputs.a2              = m_readBuffer[ 7] + (static_cast<int>((m_readBuffer[ 8]>>6)&3)<<8);
  inputs.az              = m_readBuffer[ 9] + (static_cast<int>((m_readBuffer[13]   )&3)<<8);
  inputs.dist1           = m_readBuffer[10] + (static_cast<int>((m_readBuffer[13]>>2)&3)<<8);
  inputs.dist2           = m_readBuffer[11] + (static_cast<int>((m_readBuffer[13]>>4)&3)<<8);
  inputs.supply          = m_readBuffer[12] + (static_cast<int>((m_readBuffer[13]>>6)&3)<<8);
  // IR code, ...
  inputs.irKey           = m_readBuffer[14] & 15;
  inputs.code2           = (m_readBuffer[14] >> 4) & 1;
  inputs.as1Digital      = (m_readBuffer[14] >> 6) & 1;
  inputs.as2Digital      = (m_readBuffer[14] >> 7) & 1;
  // installed modules     m_readBuffer[15] (not to implement here)
  inputs.timeInMs        = m_readBuffer[16];
  inputs.ax[FT_SLAVE1]   = m_readBuffer[17] + (static_cast<int>((m_readBuffer[20]   )&3)<<8);
  inputs.ax[FT_SLAVE2]   = m_readBuffer[18] + (static_cast<int>((m_readBuffer[20]>>2)&3)<<8);
  inputs.ax[FT_SLAVE3]   = m_readBuffer[19] + (static_cast<int>((m_readBuffer[20]>>4)&3)<<8);
  
  /* we are unable to detect extensions: 
    the following code does not work properly
    // update entensions
    const unsigned char extensions = buffer[15];
  _info.slavePresent[0] = ((extensions & EXT_S1)!=0);
  _info.slavePresent[1] = ((extensions & EXT_S2)!=0);
  _info.slavePresent[2] = ((extensions & EXT_S3)!=0);
  _info.radioCommModulePresent = ((extensions & EXT_RADIO_COMM)!=0);
  _info.internetModulePresent  = ((extensions & EXT_INTERNET)!=0); */
}


} // namespace
#endif
// EOF
