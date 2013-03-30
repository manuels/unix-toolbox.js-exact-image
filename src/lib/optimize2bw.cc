/*
 * Copyright (C) 2005 - 2010 René Rebe, ExactCODE GmbH
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

/* Short Description:
 *   Any color-space to b/w (1-bit) optimization. Applications involving
 *   huge amount of document archiving often prefer storing the data
 *   as (compressed) b/w data even in the age of TB storage as higher
 *   (color) image often explores the data volume exponentially.
 *
 *   For this case this algorithm automatically determines a threshold
 *   and performs some optimizations in order to loose as few details
 *   as possible (e.g. retrieve hand notes, stamps, etc. but leave
 *   the background color, coffee sparkels, dust and folding lines
 *   out.
 *
 *   While the first version only performed a unsharp mask and auto-
 *   thresholding, the latest version does perform a segmentation-pass,
 *   to optimize the performance (by leaving out empty areas) and
 *   to keep dithering of image areas.
 */

#include <cstdlib>
#include <cmath>

#include <iostream>
#include <iomanip>

#include "Image.hh"

#include "Colorspace.hh"
#include "Matrix.hh"

#include "optimize2bw.hh"

//#include "Timer.cc"

void optimize2bw (Image& image, int low, int high, int threshold,
		  int sloppy_threshold,
		  int radius, double standard_deviation)
{
  // do nothing if already at s/w, ...
  if (image.spp == 1 && image.bps == 1)
    return;
  
  /* Convert to RGB to gray.
     If the threshold is to be determined automatically, use color info. */
  
  const bool debug = false;
  
  // color normalize on background color
  // search for background color
  {
    int histogram[256][3] = { {0}, {0} };
    
    colorspace_by_name (image, "rgb8");
    
    uint8_t* it = image.getRawData ();
    uint8_t* end = image.getRawDataEnd ();
    
    while (it != end) {
      uint8_t r = *it++;
      uint8_t g = *it++;
      uint8_t b = *it++;
      histogram[r][0]++;
      histogram[g][1]++;
      histogram[b][2]++;
    }
    
    int lowest = 255, highest = 0, bg_r = 0, bg_g = 0, bg_b = 0;
    for (int i = 0; i <= 255; i++)
      {
	int r = histogram[i][0];
	int g = histogram[i][1];
	int b = histogram[i][2];
	
	if (debug)
	  std::cout << i << ": "<< r << " " << g << " " << b << std::endl;
	const int magic = 2; // magic denoise constant
	if (r >= magic || g >= magic || b >= magic)
	  {
	    if (i < lowest)
	      lowest = i;
	    if (i > highest)
	      highest = i;
	  }
	
	if (histogram[i][0] > histogram[bg_r][0])
	  bg_r = i;
	if (histogram[i][1] > histogram[bg_g][1])
	  bg_g = i;
	if (histogram[i][2] > histogram[bg_b][2])
	  bg_b = i;
      }
    highest = (int) (.21267 * bg_r + .71516 * bg_g + .07217 * bg_b);
    
    if (false)
    std::cerr << "lowest: " << lowest << ", highest: " << highest
	      << ", back rgb: " << bg_r << " " <<  bg_g << " " << bg_b
	      << std::endl;
    
    const int min_delta = 128;
    lowest = std::max (std::min (lowest, highest - min_delta), 0);
    highest = std::min (std::max (highest, lowest + min_delta), 255);
    
    if (low)
      lowest = low;
    if (high)
      highest = high;

    if (false)
    std::cerr << "after limit and overwrite, lowest: " << lowest
	      << ", highest: " << highest << std::endl;
    
    signed int a = (255 * 256) / (highest - lowest);
    signed int b = (-a * lowest);
    
    if (false)
    std::cerr << "a: " << (float) a / 256
	      << " b: " << (float) b / 256 << std::endl;

    it = image.getRawData ();
    end = image.getRawDataEnd ();
    uint8_t*it2 = it;
    
    while (it != end) {
      int _r = *it++;
      int _g = *it++;
      int _b = *it++;
      
      _r = (_r * a + b) / 256;
      _g = (_g * a + b) / 256;
      _b = (_b * a + b) / 256;
      
      // clip
      _r = std::max (std::min (_r, 255), 0);
      _g = std::max (std::min (_g, 255), 0);
      _b = std::max (std::min (_b, 255), 0);
      
      // on-the-fly convert to gray with associated weighting
      *it2++ = (_r*28 + _g*59 + _b*11) / 100;
    }
    
    image.spp = 1; // converted data RGB8->GRAY8
    image.setRawData();
  }

  // Convolution Matrix (unsharp mask a-like)
  if (radius > 0)
  {
    // compute kernel (convolution matrix to move over the iamge)
    // Utility::AutoTimer<Utility::Timer> timer ("convolution");
    
    matrix_type divisor = 0;
    float sd = standard_deviation;
    
    matrix_type matrix[radius+1];
    matrix_type matrix_2[radius+1];

    for (int d = 0; d <= radius; ++d) {
      matrix_type v = (matrix_type) (exp (-((float)d*d) / (2. * sd * sd)) );
      matrix[d] = v;
      divisor+=v;
      if (d>0)
	divisor+=v;
    }

    // normalize (will not work with integer matrix type !)
    divisor=1.0/divisor;
    for (int i=0; i<=radius; i++) {
      matrix[i]*=divisor;
      matrix_2[i]=-matrix[i];
    }
    
    decomposable_sym_convolution_matrix (image, matrix, matrix_2, radius, radius, 2.0);
  }
}
