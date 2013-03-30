/*
 * C++ PCX library.
 * Copyright (C) 2008 - 2009 René Rebe, ExactCODE GmbH Germany
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2. A copy of the GNU General
 * Public License can be found in the file LICENSE.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANT-
 * ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * Alternatively, commercial licensing options are available from the
 * copyright holder ExactCODE GmbH Germany.
 *
 */

#include <iostream>

#include "tga.hh"

#include "Colorspace.hh"

#include <string.h>
#include <ctype.h>

#include <Endianess.hh>
#include <inttypes.h>

using Exact::EndianessConverter;
using Exact::LittleEndianTraits;

/*
  Header
  Footer
 */

typedef struct
{
  uint8_t IDLength;
  uint8_t ColorMapType;
  uint8_t ImageType;
  EndianessConverter<uint16_t,LittleEndianTraits> ColorMapIndex;
  EndianessConverter<uint16_t,LittleEndianTraits> ColorMapLength;
  uint8_t ColorMapEntrySize;
  
  EndianessConverter<uint16_t,LittleEndianTraits> ImageXOrigin;
  EndianessConverter<uint16_t,LittleEndianTraits> ImageYOrigin;
  EndianessConverter<uint16_t,LittleEndianTraits> ImageWidth;
  EndianessConverter<uint16_t,LittleEndianTraits> ImageHeight;
  uint8_t ImageDepth;
  uint8_t ImageDescriptor;
  
} __attribute__((packed)) TGAHeader;


typedef struct
{
  EndianessConverter<uint32_t,LittleEndianTraits> ExtensionOffset;
  EndianessConverter<uint32_t,LittleEndianTraits> DeveloperOffset;
  
  char Signature[16];
  uint8_t ReservedCharacter;
  uint8_t ZeroTerminator;
} __attribute__((packed)) TGAFooter;


int TGACodec::readImage(std::istream* stream, Image& image, const std::string& decompres)
{
  bool rle = false;

  /*
  if (!stream->seekg(26, std::ios::end))
    return false;
  
  TGAFooter footer;
  
  if (!stream->read((char*)&footer, sizeof(footer)))
    return false;
  
  */
  
  TGAHeader header;
  if (!stream->read((char*)&header, sizeof(header))) {
    goto no_tga;
  }
  
  // TODO: differentiate between "maybe be old TGA" and definetly
  //       should be new-style TGA with footer

  switch (header.ImageDepth)
    {
    case 1:
    case 8:
    case 16:
    case 24:
    case 32:
      break;
    default:
      goto no_tga;
    };

  // TODO: harden checks for "maybe TGA", ...
  
  switch (header.ImageType)
    {
    case 9: // RLE, color mapped
    case 10: // RLE, true color
      rle = true;
    case 1: // color mapped
    case 2: // true color
      image.spp = 3;
      break;
    
    case 11: // RLE, b&w
      rle = true;
    case 3: // b&w
      image.spp = 1;
      break;
      
    default:
      goto no_tga;
    }

  std::cerr << "TGA: " << (int)header.IDLength << ", " << (int)header.ImageType
            << ", " << (int)header.ImageDepth << ", " << (int)header.ColorMapType
	    << ", " << header.ImageWidth << ", " << header.ImageHeight
	    << ", " << header.ImageDescriptor << std::endl;
  
  if (header.ImageDepth == 32)
    image.spp = 4;
  
  image.bps = header.ImageDepth / image.spp;
  image.setResolution(0, 0); // TODO
  
  image.resize(header.ImageWidth, header.ImageHeight);
  
  if (header.ColorMapType == 1)
    {
      // seek to color map
      stream->seekg(sizeof(header) + header.IDLength);
    }
  
  // seek to image data
  stream->seekg(sizeof(header) + header.IDLength);
  
  if (!rle) {
    // TODO: interpret flip/flop bits
    stream->read((char*)image.getRawData(), image.stride() * image.h);
  } else {
    const int bytes = header.ImageDepth / 8;
    uint8_t* data = image.getRawData();
    uint8_t payload[4];
    
    // TODO: I got one RLE which shows up RGB reversed?!
    for (int i = 0; i < image.stride() * image.h;)
      {
	uint8_t t = stream->get();
	uint8_t n = (t & 0x7f) + 1;
	if (t & 0x80) // RLE
	  {
	    stream->read((char*)payload, bytes);
	    while (n-- > 0 && i < image.stride() * image.h)
	      for (int j = 0; j < bytes; ++j)
		data[i++] = payload[j];
	  }
	else // RAW
	  {
	    stream->read((char*)data + i, n * bytes);
	    i += n * bytes;
	  }
      }
  }
  
  {
    unsigned ori = (header.ImageDescriptor >> 4) & 2;
    if (ori != 2)
      std::cerr << "unimplemented TGA orientation: " << ori << std::endl;
  }
  
  return true;
  
 no_tga:
  stream->seekg(0);
  return false;
}

bool TGACodec::writeImage(std::ostream* stream, Image& image, int quality,
			  const std::string& compress)
{
  TGAHeader header;
  header.IDLength = 0;
  header.ColorMapType = 0;
  header.ImageType = image.spp == 1 ? 3 /*Truecolor*/ : 2/*bw*/;
  header.ColorMapIndex = 0;
  header.ColorMapLength = 0;
  header.ColorMapEntrySize = 0;
  
  header.ImageXOrigin = 0;
  header.ImageYOrigin = 0;
  header.ImageWidth = image.width();
  header.ImageHeight = image.height();
  header.ImageDepth = image.spp * image.bps;
  header.ImageDescriptor = 0x20; // top-left
  
  stream->write((char*)&header, sizeof(header));
  
  stream->write((char*)image.getRawData(), image.stride() * image.height());
  
  TGAFooter footer;
  footer.ExtensionOffset = 0;
  footer.DeveloperOffset = 0;
  
  strcpy(footer.Signature, "TRUEVISION-XFILE");
  footer.ReservedCharacter = '.';
  footer.ZeroTerminator = 0;
  stream->write((char*)&footer, sizeof(footer));
  
  return true;
}

TGACodec tga_codec;
