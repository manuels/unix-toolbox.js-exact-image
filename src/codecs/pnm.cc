/*
 * Copyright (C) 2006 - 2010 René Rebe
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
 */

/*
 * C++ PNM library.
 *
 * I former times we used to use the netbpm library here, but as
 * it can not handle in memory or otherwise transferred files, allowing
 * access via the C FILE* exclusively (???!!!) we had to write our
 * own parser here ...
 *
 */

#include <string.h> // memcpy

#include <iostream>
#include <string>
#include <sstream>

#include "pnm.hh"
#include "Endianess.hh"
using namespace Exact;

int getNextHeaderNumber (std::istream* stream)
{
  for (bool whitespace = true; whitespace && stream;)
    {
      int c = stream->peek();
      switch (c) {
      case ' ':
	stream->get(); // consume silently
	break;
      case '\n':
      case '\r':
	stream->get(); // consume silently
	// comment line?
	while (stream->peek() == '#') {
	  std::string str;
	  std::getline(*stream, str); // consume comment line
	}
	break;
      default:
	whitespace = false;
      }
    }
  
  int i;
  *stream >> i;
  return i;
}

int PNMCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
  // check signature
  if (stream->peek () != 'P')
    return false;
  stream->get(); // consume P
  
  image.bps = 0;
  char mode = stream->peek();
  switch (mode) {
  case '1':
  case '4':
    image.bps = 1;
  case '2':
  case '5':
    image.spp = 1;
    break;
  case '3':
  case '6':
    image.spp = 3;
    break;
  default:
    stream->unget(); // P
    return false;
  }
  stream->get(); // consume format number
  
  image.w = getNextHeaderNumber (stream);
  image.h = getNextHeaderNumber (stream);
  
  int maxval = 1;
  if (image.bps != 1) {
    maxval = getNextHeaderNumber (stream);
  }
  
  image.bps = 1;
  while ((1 << image.bps) < maxval)
    ++image.bps;
  
  // not stored in the format :-(
  image.setResolution(0, 0);
  
  // allocate data, if necessary
  image.resize (image.w, image.h);
  
  // consume the left over spaces and newline 'till the data begins
  {
    std::string str;
    std::getline (*stream, str);
  }
  
  if (mode <= '3') // ascii / plain text
    {
      Image::iterator it = image.begin ();
      for (int y = 0; y < image.h; ++y)
	{
	  for (int x = 0; x < image.w; ++x)
	    {
	      if (image.spp == 1) {
		int i;
		*stream >> i;
		
		i = i * (255 / maxval);

		// only mode 1 is defined with 1 == black, ...
		if (mode == '1') i = 255 - i;

		it.setL (i);
	      }
	      else {
		uint16_t r, g, b;
		*stream >> r >> g >> b;
		
		it.setRGB (r, g, b);
	      }
	      
	      it.set (it);
	      ++it;
	    }
	}
    }
  else // binary data
    {
      const int stride = image.stride ();
      const int bps = image.bps;
      
      for (int y = 0; y < image.h; ++y)
	{
	  uint8_t* dest = image.getRawData() + y * stride;
	  stream->read ((char*)dest, stride);

	  // is it publically defined somewhere???
	  if (bps == 1) {
	    uint8_t* xor_ptr = dest;
	    for (int x = 0; x < image.w; x += 8)
	      *xor_ptr++ ^= 0xff;
	  }
	  
	  if (bps == 16) {
	    uint16_t* swap_ptr = (uint16_t*)dest;
	    for (int x = 0; x < stride/2; ++x, ++swap_ptr)
	      *swap_ptr = ByteSwap<NativeEndianTraits,BigEndianTraits, uint16_t>::Swap (*swap_ptr);
	  }
	}
    }
  
  return true;
}

bool PNMCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
  // ok writing should be easy ,-) just dump the header
  // and the data thereafter ,-)
  
  int format = 0;
  
  if (image.spp == 1 && image.bps == 1)
    format = 1;
  else if (image.spp == 1)
    format = 2;
  else if (image.spp == 3)
    format = 3;
  else {
    std::cerr << "Not (yet?) supported PBM format." << std::endl;
    return false;
  }
  
  std::string c (compress);
  std::transform (c.begin(), c.end(), c.begin(), tolower);
  if (c == "plain")
    c = "ascii";

  
  if (c != "ascii")
    format += 3;
  
  *stream << "P" << format << std::endl;
  *stream << "# http://exactcode.com/oss/exactimage/" << std::endl;

  *stream << image.w << " " << image.h << std::endl;
  
  // maxval
  int maxval = (1 << image.bps) - 1;
  
  if (image.bps > 1)
    *stream << maxval << std::endl;
  
  Image::iterator it = image.begin ();
  if (c == "ascii")
    {
      for (int y = 0; y < image.h; ++y)
	{
	  for (int x = 0; x < image.w; ++x)
	    {
	      *it;
	      
	      if (x != 0)
		*stream << " ";
	      
	      if (image.spp == 1) {
		int i = it.getL();

		// only mode 1 is defined with 1 == black, ...
                if (format == 1) i = 255 - i;

		*stream << i / (255 / maxval);
	      }
	      else {
		uint16_t r = 0, g = 0, b = 0;
		it.getRGB (&r, &g, &b);
		*stream << (int)r << " " << (int)g << " " << (int)b;
	      }
	      ++it;
	    }
	  *stream << std::endl;
	}
    }
  else
    {
      const int stride = image.stride ();
      const int bps = image.bps;
      
      uint8_t* ptr = (uint8_t*) malloc (stride);
      
      for (int y = 0; y < image.h; ++y)
	{
	  memcpy (ptr, image.getRawData() + y * stride, stride);
	  
	  // is this publically defined somewhere???
	  if (bps == 1) {
	    uint8_t* xor_ptr = ptr;
	    for (int x = 0; x < image.w; x += 8)
	      *xor_ptr++ ^= 0xff;
	  }
	  
	  if (bps == 16) {
	    uint16_t* swap_ptr = (uint16_t*)ptr;
	    for (int x = 0; x < stride/2; ++x, ++swap_ptr)
	      *swap_ptr = ByteSwap<BigEndianTraits, NativeEndianTraits, uint16_t>::Swap (*swap_ptr);
	  }
	  
	  stream->write ((char*)ptr, stride);
	}
      free (ptr);
    }
  
  stream->flush ();
  
  return true;
}

PNMCodec pnm_loader;
