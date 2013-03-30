/*
 * Copyright (C) 2006 - 2011 René Rebe
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
 * All rights reserved. Commercial licensing options are
 * available from the copyright holder ExactCODE GmbH Germany.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "raw.hh"

int RAWCodec::readImage(std::istream* stream, Image& image, const std::string& decompres)
{
  if (image.w <= 0 || image.bps == 0 || image.spp == 0) {
    std::cerr << "RAWCodec: image parameters not sufficently defined!" << std::endl;
    return false;
  }

  int h = image.h;
  if (h > 0) // if we know the height up-front
    image.resize(image.w, image.h);
  
  int y = 0;
  for (y = 0; h <= 0 || y < h; ++y)
    {
      if (h <= 0) // height not known up-front, resize line by line
	image.resize (image.w, y + 1);
	
      stream->read((char*)image.getRawData() + image.stride() * y,
		   image.stride());
      if (!stream->good())
	break;
    }
  
  if (h > 0)
    {
      if (y != h) {
	std::cerr << "RAWCodec: Error reading line: " << y << std::endl;
	return false;
      }
      return true;
    }
  else
    {
      if (y == 0) {
	std::cerr << "RAWCodec: Error reading a line of image with undefined height at all (stride: " << image.stride() << ")" << std::endl;
	return false;
      }
      image.resize (image.w, y - 1); // final size of scanlines fully read
      return true;
    }
}

bool RAWCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
  if (!image.getRawData())
    return false;

  return stream->write ((char*)image.getRawData(), image.stride()*image.h)
    /* ==
       (size_t) image.stride()*image.h*/;
}

RAWCodec raw_loader;
