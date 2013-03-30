/*
 * C++ PCX library.
 * Copyright (C) 2008 - 2010 René Rebe, ExactCODE GmbH Germany
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

#include <string.h>
#include <ctype.h>

#include "pcx.hh"

#include "Colorspace.hh"
#include "Endianess.hh"
#include "inttypes.h"

using Exact::EndianessConverter;
using Exact::LittleEndianTraits;

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct
{
  uint8_t Manufacturer;
  uint8_t Version;
  uint8_t Encoding;
  uint8_t BitsPerPixel;
  
  EndianessConverter<uint16_t,LittleEndianTraits> WindowXmin;
  EndianessConverter<uint16_t,LittleEndianTraits> WindowYmin;
  
  EndianessConverter<uint16_t,LittleEndianTraits> WindowXmax;
  EndianessConverter<uint16_t,LittleEndianTraits> WindowYmax;
  
  EndianessConverter<uint16_t,LittleEndianTraits> HDpi;
  EndianessConverter<uint16_t,LittleEndianTraits> VDpi;
  
  uint8_t Colormap[16][3];
  uint8_t Reserved;
  
  uint8_t NPlanes;
  
  EndianessConverter<uint16_t,LittleEndianTraits> BytesPerLine;
  // 0: udef, 1: color, 2: grayscale
  EndianessConverter<uint16_t,LittleEndianTraits> PaletteInfo;

  EndianessConverter<uint16_t,LittleEndianTraits> HScreenSize;
  EndianessConverter<uint16_t,LittleEndianTraits> VScreenSize;

  uint8_t Filler[54];
}
#ifdef __GNUC__
__attribute__((packed))
#endif
PCXHeader;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

int PCXCodec::readImage(std::istream* stream,
			 Image& image, const std::string& decompres)
{
  if (stream->peek() != 0x0a)
    return false;
  stream->get();
  if ((unsigned)stream->peek() > 5) {
    stream->unget();
    return false;
  }
  stream->unget();
  
  PCXHeader header;
  if (!stream->read((char*)&header, sizeof(header)))
    return false;

  switch (header.BitsPerPixel)
   {
   case 1:
   case 8:
   case 16:
   case 24:
   case 32:
     break;
   default:
     std::cerr << "PCX invalid bit-depth: " << header.BitsPerPixel << std::endl;
     goto no_pcx;
   };

  switch (header.NPlanes)
   {
   case 1:
   case 3:
   case 4:
     break;
   default:
     std::cerr << "PCX invalid plane count: " << header.NPlanes << std::endl;
     goto no_pcx;
   };
  
  image.bps = header.BitsPerPixel;
  image.spp = header.NPlanes;
  if (image.spp == 4 && image.bps == 1)
    image.spp = 1, image.bps = 4;
  image.setResolution(header.HDpi, header.VDpi);
  
  image.resize(header.WindowXmax - header.WindowXmin + 1,
	       header.WindowYmax - header.WindowYmin + 1);
  std::cerr << image.w << "x" << image.h << std::endl;
  std::cerr << "Version: " << (int)header.Version
	    << ", PaletteInfo: " << header.PaletteInfo << std::endl;
  std::cerr << "BitsPerPixel: " << (int)header.BitsPerPixel
	    << ", NPlanes: " << (int)header.NPlanes << std::endl;
  std::cerr << "BytesPerLine: " << (int)header.BytesPerLine << std::endl;
  std::cerr << "Encoding: " << (int)header.Encoding << std::endl;
  
  // TODO: more buffer checks, palette handling
  
  
  {
    const bool plane_packing = header.NPlanes > 1;
    uint8_t* scanline = (plane_packing ?
			 new uint8_t[header.BytesPerLine * header.NPlanes] :
			 image.getRawData());
    for (int y = 0; y < image.h; ++y)
      {
	for (int i = 0; i < header.BytesPerLine * header.NPlanes;)
	  {
	    uint8_t n = 1, v = stream->get();
	    if ((header.Encoding == 1) && (v & 0xC0) == 0xC0) {
	      n = v ^ 0xC0;
	      v = stream->get();
	    }
	    
	    while (n-- > 0 && i < header.BytesPerLine * header.NPlanes)
	      {
		scanline[i++] = v;
	      }
	  }
	
	if (plane_packing) // re-pack planes
	  {
            const unsigned int bits = header.BitsPerPixel;
	    const unsigned int planes = header.NPlanes;
	    uint8_t* dst = image.getRawData() + image.stride() * y;
	    
	    struct BitPacker {
	      uint8_t* ptr;
	      int p;
	      const uint8_t bits, mask;
	      const bool msb_first;
	      
	      BitPacker(uint8_t* _ptr, int _bits, bool _msb_first = true)
		: ptr(_ptr), p(_msb_first ? 8 - _bits : 0),
		  bits(_bits), mask((1 << _bits) - 1), msb_first(_msb_first)
	      {
	      }
	      
	      void push(uint8_t v)
	      {
		if (!msb_first) {
		  *ptr = (*ptr & ~(mask << p)) | ((v & mask) << p);
		  p += bits;
		  if (p >= 8)
		    p = 0, ++ptr;
		} else {
		  *ptr = (*ptr & ~(mask << p)) | ((v & mask) << p);
		  p -= bits;
		  if (p < 0)
		    p = 8 - bits, ++ptr;
		}
	      }
	    };
	    BitPacker packer(dst, bits == 8 ? bits : bits * planes);
	    
	    for (int i = 0; i < image.w; ++i)
	      {
		uint8_t v = 0, mask = (1 << bits) - 1;
		int b = 0;
		for (int p = 0; p < planes; ++p)
		  {
		    uint8_t* src = scanline + p * header.BytesPerLine;
		    int idx = i * bits / 8;
		    int bit = i * bits % 8;
		    bit = 8 - bits - bit;
		    v |= ((src[idx] >> bit) & mask) << b;
                    b += bits;
		    if (b >= (bits == 8 ? bits : bits * planes)) {
		      packer.push(v);
		      b = v = 0;
		    }
		  }
	      }
	  }
	else // in-memory write
	  {
	    scanline += image.stride();
	  }
      }
    if (plane_packing)
      delete[] scanline;
  }

  if (image.spp == 1)
  {
    uint16_t rmap[256];
    uint16_t gmap[256];
    uint16_t bmap[256];
    
    int colormap_magic = stream->get();
    if (colormap_magic == 0x0c) {
      std::cerr << "reading colormap" << std::endl;
      for (int i = 0; i < 256; ++i)
	{
	  uint8_t entry[3];
	  stream->read((char*)entry, sizeof(entry));
	  rmap[i] = entry[0] * 0xffff / 0xff;
	  gmap[i] = entry[1] * 0xffff / 0xff;
	  bmap[i] = entry[2] * 0xffff / 0xff;
	}
      if (!stream->good()) {
	std::cerr << "error reading PCX colormap" << std::endl;
	return false;
      }
      
      colorspace_de_palette (image, 256, rmap, gmap, bmap);
    }
    else if (header.PaletteInfo == 1 || header.PaletteInfo == 2)
      {
	const int ncolors = 1 << image.bps;
	for (int i = 0; i < ncolors; ++i)
	  {
	    rmap[i] = header.Colormap[i][0] * 0xffff / 0xff;
	    gmap[i] = header.Colormap[i][1] * 0xffff / 0xff;
	    bmap[i] = header.Colormap[i][2] * 0xffff / 0xff;
	  }
	colorspace_de_palette(image, ncolors, rmap, gmap, bmap);
      }
  }
  
  return true;
  
no_pcx:
  stream->seekg(0);
  return false;
}

bool PCXCodec::writeImage(std::ostream* stream, Image& image, int quality,
			  const std::string& compress)
{
  PCXHeader header;
  header.Manufacturer = 10;
  header.Version = 5; // TODO: save older versions if not RGB
  
  header.Encoding = 0; // 1: RLE
  header.NPlanes = image.spp;
  header.BytesPerLine = image.stride() / image.spp;
  header.BitsPerPixel = image.bps;
  header.PaletteInfo = 0;
  
  switch (header.BitsPerPixel)
    {
    case 1:
    case 8:
    case 16:
    case 24:
    case 32:
     break;
    default:
      std::cerr << "unsupported PCX bit-depth" << std::endl;
      return false;
   };
  
  header.HDpi = image.resolutionX();
  header.VDpi = image.resolutionY();
  header.WindowXmin = 0;
  header.WindowYmin = 0;
  header.WindowXmax = image.width() - 1;
  header.WindowYmax = image.height() - 1;
  
  stream->write((char*)&header, sizeof(header));
  
  // write "un"compressed image data
  // TODO: RLE compress
  for (int y = 0; y < image.h; ++y)
    {
      for (int plane = 0; plane < image.spp; ++plane)
	{
	  uint8_t* data = image.getRawData() + image.stride() * y + plane;
	  for (int x = 0; x < image.w; ++x)
	    {
	      stream->write((char*)data, 1);
	      data += image.spp;
	    }
	}
    }
  
  return true;
}

PCXCodec pcx_codec;
