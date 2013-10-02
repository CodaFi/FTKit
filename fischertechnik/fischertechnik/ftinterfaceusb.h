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
#ifndef FT_INTERFACE_USB_H
#define FT_INTERFACE_USB_H

#include "ftapi.h"

namespace ftapi {

  class FtInterfaceUsb: public FtInterface {
    protected:
    static const int ROBO_USB_VENDOR_ID = 0x146a;
    static const int ROBO_IF_USB_PRODUCT_ID = 0x0001;
    static const int ROBO_IOE_USB_PRODUCT_ID = 0x0002;
    static const int ROBO_RFD_USB_PRODUCT_ID = 0x0003;
    static const int ROBO_SAL_USB_PRODUCT_ID = 0x0005; // sound + lights

    static const unsigned char ROBO_IF_OUT_EP = 0x1;
    static const unsigned char ROBO_IF_IN_EP = 0x81;
    static const unsigned char ROBO_IF_COMPLETE = 0x8A; // old: 0xF2;
    static const unsigned char ROBO_IF_COMPLETE_WRITE_SIZE = 17;
    static const unsigned char ROBO_IF_COMPLETE_READ_SIZE = 27; // old: 21
    static const unsigned char ROBO_IF_INTERN = 0xF0;
    static const unsigned char ROBO_IF_INTERN_REV = 1;
    static const unsigned char ROBO_IF_INTERN_SN = 2;
    static const unsigned char ROBO_IF_INTERN_MODE = 3;
    static const unsigned char ROBO_IF_FLASH_QUERYNAME = 0xFA;
    static const unsigned char ROBO_IF_FLASH_START = 0x12;
    static const unsigned char ROBO_IF_FLASH_STOP = 0x13;
    static const unsigned char ROBO_IOE_COMPLETE_WRITE_SIZE = 6;
    static const unsigned char ROBO_IOE_COMPLETE_READ_SIZE = 6;
    static const unsigned char ROBO_IOE_MARKING = 0xF0;
    static const unsigned char ROBO_RFD_SPECIAL = 0xFB;
    static const unsigned char ROBO_RFD_INTERN_REV = 0x81;
    static const unsigned char ROBO_RFD_RF_ON = 0xFB;
    static const unsigned char ROBO_RFD_RF_OFF = 0x21;
    static const unsigned int  ROBO_RFD_RF = 0x102;
    static const unsigned char ROBO_RFD_IF_COMPLETE = 0x8A;
    static const unsigned char ROBO_RFD_IF_COMPLETE_WRITE_SIZE = 18;
    static const unsigned char ROBO_RFD_IF_COMPLETE_READ_SIZE = 28;
    static const unsigned char ROBO_RFD_IF_OUT_EP = 0x2;
    static const unsigned char ROBO_RFD_IF_IN_EP = 0x82;
 

    // constants for extension bits
/*    static const unsigned char EXT_S1 = 1;
    static const unsigned char EXT_S2 = 2;
    static const unsigned char EXT_S3 = 4;
    static const unsigned char EXT_RADIO_COMM = 16;
    static const unsigned char EXT_INTERNET   = 32;*/

    // prepare packet to send to ROBO Interface (ROBO_IF_COMPLETE).
    // @note buffer needs to be at least 17 bytes long
    IfInputs roboIfCopyFromBuffer(unsigned char *buffer);
    // extract data from packed received from ROBO Interface (ROBO_IF_COMPLETE).
    // @note buffer needs to be at least 21 bytes long
    void roboIfCopyToBuffer(const IfOutputs &outputs, unsigned char *buffer);

    // prepare packet to send to ROBO I/O Extension (ROBO_IF_COMPLETE).
    // @note buffer needs to be at least 6 bytes long
    IfInputs roboIoeCopyFromBuffer(unsigned char *buffer);
    // extract data from packed received from ROBO I/O Extension (ROBO_IF_COMPLETE).
    // @note buffer needs to be at least 6 bytes long
    void roboIoeCopyToBuffer(const IfOutputs &outputs, unsigned char *buffer);
 };

} // namespace ftapi
#endif
// EOF
