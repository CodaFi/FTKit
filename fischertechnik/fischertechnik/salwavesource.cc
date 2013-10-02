/***************************************************************************
 * ftApi - fischertechnik c++ progamming api                               *
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
#include "salwavesource.h"
#include <fstream>
#include <exception>
#include <ftapi.h>
#include <cassert>
#include <iostream>

namespace ftapi {

SalWaveSource::SalWaveSource(std::string fn): _stream(fn.c_str(),  std::fstream::in) {
  // header
  if (!_stream.good()) throw XFt("cannot open file \""+fn+"\"");
  if ('R'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('I'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('F'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('F'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  unsigned long wavlen = 0;
  for (int i=0; i<4; ++i) wavlen = wavlen | (_readByte() << 8*i);
  if ('W'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('A'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('V'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('E'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");

//  std::cout << "wav len " << std::dec << wavlen << std::endl;
  
  // format chunk
  if ('f'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('m'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if ('t'!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  if (' '!=_readByte()) throw XFt("cannot read header from file \""+fn+"\"");
  
  unsigned long fmtlen = 0;
  for (int i=0; i<4; ++i) fmtlen = fmtlen | (_readByte() << 8*i);
//  std::cout << "fmt len " << std::dec << fmtlen << std::endl;
  
  unsigned short wFormatTag = 0;
  for (int i=0; i<2; ++i) wFormatTag = wFormatTag | (_readByte() << 8*i);
  unsigned short wChannels = 0;
  for (int i=0; i<2; ++i) wChannels = wChannels | (_readByte() << 8*i);
  unsigned long dwSamplesPerSec = 0;
  for (int i=0; i<4; ++i) dwSamplesPerSec = dwSamplesPerSec | (_readByte() << 8*i);
  
  if (1!=wFormatTag) throw XFt("wav format not supported reading \""+fn+"\" - please supply 8bit 22khz pcm 1 channel");
  if (1!=wChannels) throw XFt("wav format not supported reading \""+fn+"\" - please supply 8bit 22khz pcm 1 channel");
  if (22050!=dwSamplesPerSec) throw XFt("wav format not supported reading \""+fn+"\" - please supply 8bit 22khz pcm 1 channel");

  // dwAvgBytesPerSec (unsigned long, nötige Übertragungsbandbreite)
  for (int i=0; i<4; ++i) _readByte();
  // wBlockAlign (unsigned short, Größe der Frames in Bytes)
  unsigned short wBlockAlign = 0;
  for (int i=0; i<2; ++i) wBlockAlign = wBlockAlign | (_readByte() << 8*i);
  if (1!=wBlockAlign) throw XFt("wav format not supported reading \""+fn+"\" - please supply 8bit 22khz pcm 1 channel");
  // Für PCM-Daten hat der Format-Chunk nur noch dieses eine Feld:
  // BitsPerSample (unsigned short, Quantisierungsauflösung, identisch für alle Kanäle)
  for (int i=0; i<2; ++i) _readByte();
  // read 16 bytes so far, account for lenght
  for (unsigned int i=16; i<fmtlen; ++i) _readByte();
  
  // next header
  unsigned long hdr = 0;
  for (int i=0; i<4; ++i) hdr = hdr | (_readByte() << 8*i);
//  std::cout << "hdr code " << std::hex << hdr  <<std::endl;
  while (0x61746164!=hdr) { // "data"
    if (!_stream.good()) throw XFt("cannot read data from file \""+fn+"\"");
    // drop unknown hdr
    unsigned long hdrlen = 0;
    for (int i=0; i<4; ++i) hdrlen = hdrlen | (_readByte() << 8*i);
    for (unsigned int i=0; i<hdrlen; ++i) _readByte();
     
    //next
    hdr = 0;
    for (int i=0; i<4; ++i) hdr = hdr | (_readByte() << 8*i);
//    std::cout << "hdr code " << std::hex << hdr  <<std::endl;
  }
  // 
  unsigned long datalen = 0;
  for (int i=0; i<4; ++i) datalen = datalen | (_readByte() << 8*i);
//  std::cout << "data len " << std::dec << datalen << std::endl;
  
  _bytesAvail = datalen;
  // read 
  
/*
Wird keine Kompression verwendet, ist dwAvgBytesPerSec das Produkt aus Abtastrate und Framegröße. Die Framegröße ergibt sich aus der Vorgabe, dass alle Werte im Daten-Chunk als Integer zu kodieren sind mit einer gerade ausreichenden Größe in Byte (evtl nötige Pad-Bits stehen am niederwertigen Ende mit dem Wert 0). Für das PCM-Format gilt

        wBlockAlign = wChannels * ((wBitsPerSample + 7) / 8) (Integer-Division ohne Rest),

sodass die Framegröße für 12-Bit-Stereo nicht drei sondern vier Byte beträgt.
*/
}

unsigned char SalWaveSource::readByte() {
  if (!_bytesAvail) throw XFt("eof");
  --_bytesAvail;
  return _readByte();
}

SalWaveSource::~SalWaveSource() {
}

unsigned char SalWaveSource::_readByte() {
  char c;
  _stream.get(c);
  if (_stream.good()) {
    return static_cast<unsigned char>(c);
  }
  return 0;
}


}
