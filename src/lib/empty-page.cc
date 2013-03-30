/*
 * Copyright (C) 2005 - 2008 René Rebe
 *           (C) 2005 - 2007 Archivista GmbH, CH-8042 Zuerich
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

#include <iostream>

#include "Image.hh"
#include "Codecs.hh"

#include "Colorspace.hh"
#include "Matrix.hh"

#include "empty-page.hh"
#include "optimize2bw.hh"

/* TODO: for more accurance one could introduce a hot-spot area that
   has a higher weight than the other (outer) region to more reliably
   detect crossed but otherwise empty pages */
bool detect_empty_page (Image& im, double percent, int margin,
			int* set_pixels)
{
  // sanitize margin
  if (margin % 8 != 0)
    margin -= margin % 8;
  
  // TODO: optimize not to copy the pixel data on colorspace conversion
  Image image;
  image = im;
  
  // already in sub-byte domain? just count the black pixels
  if (image.spp == 1 && (image.bps > 1 && image.bps < 8))
    {
      colorspace_by_name (image, "gray1");
    }
  // if not 1-bit, yet: convert it down ...
  else if (image.spp != 1 || image.bps != 1) {
    // don't care about cmyk vs. rgb, just get gray8 pixels, quickly
    colorspace_by_name (image, "gray8");

    // force quick pass, no color use, 1px radius
    optimize2bw (image, 0, 0, 128, 0, 1);
    // convert to 1-bit (threshold) - optimize2bw does not perform that step ...
    colorspace_gray8_to_gray1 (image);
  }
  
  // count bits and decide based on that
  
  // create a fast bit count lookup table
  int bits_set[256] = { 0 };
  for (int i = 0; i < 256; i++) {
    int bits = 0;
    for (int j = i; j != 0; j >>= 1) {
      bits += (j & 0x01);
    }
    bits_set[i] = bits;
  }
  
  int stride = (image.w * image.bps * image.spp + 7) / 8;
  
  // count pixels by table lookup
  int pixels = 0;
  uint8_t* data = image.getRawData();
  for (int row = margin; row < image.h-margin; row++) {
    for (int x = margin/8; x < stride - margin/8; x++) {
      int b = bits_set [ data[stride*row + x] ];
      // it is a bits_set table - and we want the zeros ...
      pixels += 8-b;
    }
  }

  float image_percent = (float)pixels/(image.w*image.h) * 100;
  
  if (set_pixels)
    *set_pixels = pixels;
  
  return image_percent < percent;
}
