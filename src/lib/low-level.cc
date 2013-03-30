/*
 * Misc. low-level helpers, usually to aid debugging.
 * Copyright (C) 2007 - 2010 Ren\xc3\xa9 Rebe, ExactCOD GmbH Germany
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

#include <stdlib.h> // malloc
#include <string.h> // memcpy

#include "low-level.hh"

void deinterlace (Image& image)
{
  const int stride = image.stride();
  const int height = image.height();
  uint8_t* deinterlaced = (uint8_t*) malloc(image.stride() * height);
  
  for (int i = 0; i < height; ++i)
    {
      const int dst_i = i / 2 + (i % 2) * (height / 2);
      std::cerr << i << " - " << dst_i << std::endl;
      uint8_t* dst = deinterlaced + stride * dst_i;
      uint8_t* src = image.getRawData() + stride * i;
      
      memcpy(dst, src, stride);
    }
  
  image.setRawData(deinterlaced);
}
