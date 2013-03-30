/*
 * Convolution Matrix.
 * Copyright (C) 2006 - 2012 René Rebe, ExactCODE GmbH Germany
 * Copyright (C) 2007 Valentin Ziegler, ExactCODE GmbH Germany
 * Copyright (C) 2007 Susanne Klaus, ExactCODE GmbH Germany
 * Copyright (C) 2006 Archivista
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

#include "Matrix.hh"
#include "Codecs.hh"

#include "ImageIterator2.hh"

template <typename T>
struct convolution_matrix_template
{
  void operator() (Image& image, const matrix_type* matrix, int xw, int yw,
		   matrix_type divisor)
  {
    Image orig_image;
    orig_image.copyTransferOwnership (image);
    image.resize (image.w, image.h);
    
    T dst_it (image);
    T src_it (orig_image);
    
    const int xr = xw / 2;
    const int yr = yw / 2;
    
    // top & bottom
    for (int y = 0; y < image.h; ++y)
      {
	for (int x = 0; x < image.w;)
	  {
	    dst_it.at (x, y);
	  
	    typename T::accu a;
	  
	    const matrix_type* _matrix = matrix;
	  
	    for (int ym = 0; ym < yw; ++ym) {
	    
	      int image_y = y-yr+ym;
	      if (image_y < 0)
		image_y = 0 - image_y;
	      else if (image_y >= image.h)
		image_y = image.h - (image_y - image.h) - 1;
	    
	      for (int xm = 0; xm < xw; ++xm) {
	      
		int image_x = x-xr+xm;
		if (image_x < 0)
		  image_x = 0 - image_x;
		else if (image_x >= image.w)
		  image_x = image.w - (image_x - image.w) - 1;
	      
		a += *(src_it.at(image_x, image_y)) * *_matrix;
		++src_it;
		++_matrix;
	      }
	    }
	    a /= divisor;
	    a.saturate ();
	  
	    dst_it.set (a);
	    ++dst_it;
	  
	    ++x;
	    if (x == xr && y >= yr && y < image.h - yr)
	      x = image.w - xr;
	  }
      }
    
    //image area without border
    for (int y = yr; y < image.h - yr; ++y)
      {
	dst_it.at (xr, y);
	for (int x = xr; x < image.w - xr; ++x)
	  {
	    typename T::accu a;
	    const matrix_type* _matrix = matrix;
	    
	    for (int ym = 0; ym < yw; ++ym) {
	      src_it.at (x - xr, y - yr + ym);
	      for (int xm = 0; xm < xw; ++xm) {
		a += *src_it * *_matrix;
		++src_it;
		++_matrix;
	      }
	    }
	    a /= divisor;
	    a.saturate();
	    
	    dst_it.set (a);
	    ++dst_it;
	  }
      }
  }
};

void convolution_matrix (Image& image, const matrix_type* m, int xw, int yw,
			 matrix_type divisor)
{
  codegen<convolution_matrix_template> (image, m, xw, yw, divisor);
}

// h_matrix contains entrys m[0]...m[xw]. It is assumed, that m[-i]=m[i]. Same for v_matrix.
void decomposable_sym_convolution_matrix (Image& image,
					  const matrix_type* h_matrix, const matrix_type* v_matrix,
					  int xw, int yw, matrix_type src_add)
{
  int bps = image.bitsPerSample();
  int spp = image.samplesPerPixel();
  int stride = image.stride();
  int height = image.height();

  // TODO: support for 16 bit
  if (bps != 8) {
    std::cerr << "sorry, convolution only supports 8 bits per sample" << std::endl;
    return;
  }

  uint8_t* data = image.getRawData();

  matrix_type* tmp_data = (matrix_type*)malloc (stride * (1+(2*yw)) * sizeof(matrix_type));
  matrix_type* line_data = (matrix_type*)malloc (std::max(stride, height) * sizeof(matrix_type));

  uint8_t* src_ptr;
  matrix_type* tmp_ptr;
  matrix_type val;

  // main transform loop
  for (int y=0; y < height+yw; ++y) {

    // horizontal transform
    if (y<height) {
      src_ptr = &data[(y * stride)];
      tmp_ptr = &tmp_data[(y%(1+(2*yw))) * stride];

      val=h_matrix[0];
      for (int x = 0; x < stride; ++x) {
	matrix_type v = (matrix_type)src_ptr[x];
	tmp_ptr[x]=val*v;
	line_data[x]=v;
      }
 
      for (int i=1; i<=xw; ++i) {
	int pi = spp*i;
	int dstart = pi;
	int dend = stride-pi;
	int end = stride;
	int l = pi;
	int r = 0;
	val = h_matrix[i];

	// left border
	for (int x = 0; x < dstart; x++, l++)
	  tmp_ptr[x] += val*line_data[l];

	// middle
	for (int x = dstart; x < dend; x++, l++, r++)
	  tmp_ptr[x] += val * (line_data[l] + line_data[r]);

	// right border
	for (int x = dend; x < end; x++, r++)
	  tmp_ptr[x] += val*line_data[r];
      }
    }

    // now do the vertical transform of a block of lines and write back to src
    int dsty = y-yw;
    if (dsty >= 0) {
      src_ptr = &data[dsty * stride];
      val = (matrix_type)src_add;
      if (val != (matrix_type)0) {
	for (int x = 0; x < stride; ++x) {
	  matrix_type v = (matrix_type)src_ptr[x];
	  line_data[x] = val*v;
	}
      } else {
	for (int x = 0; x < stride; ++x)
	  line_data[x] = 0.0;
      }

      for (int i = 0; i <= yw; i++) {
	val = v_matrix[i];
	if (i == 0 || (dsty-i < 0) || (dsty+i >= height) ) {
	  int tmpy = (dsty-i<0) ? dsty+i : dsty-i;
	  tmp_ptr = &tmp_data[(tmpy%(1+(2*yw))) * stride];
	  for (int x = 0; x < stride; ++x)
	    line_data[x] += val * tmp_ptr[x];

	} else {
	  tmp_ptr = &tmp_data[((dsty-i)%(1+(2*yw))) * stride];
	  matrix_type* tmp_ptr2 = &tmp_data[((dsty+i)%(1+(2*yw))) * stride];
	  for (int x = 0; x < stride; ++x)
	    line_data[x] += val * (tmp_ptr[x] + tmp_ptr2[x]);
	}
      }
      
      for  (int x = 0; x < stride; ++x) {
	uint8_t z = (uint8_t)(line_data[x] > 255 ? 255 : line_data[x] < 0 ? 0 : line_data[x]);
	src_ptr[x] = z;
      }
    }
  }

  image.setRawData(); // invalidate as altered
  free(tmp_data);
  free(line_data);
}


void decomposable_convolution_matrix (Image& image,
				      const matrix_type* h_matrix, const matrix_type* v_matrix,
				      int xw, int yw,
				      matrix_type src_add)
{
  uint8_t* data = image.getRawData();
  matrix_type* tmp_data = (matrix_type*) malloc (image.w * image.h * sizeof(matrix_type));
  
  const int xr = xw / 2;
  const int yr = yw / 2;
  const int xmax = image.w - ((xw+1)/2);
  const int ymax = image.h - ((yw+1)/2);

  uint8_t* src_ptr = data;
  matrix_type* tmp_ptr = tmp_data;

  // valentin 2007-10-01: i think horizontal and vertical are accidentally switched in those comments ???

  // transform the vertical convolution strip with h_matrix, leaving out the left/right borders
  for (int y = 0; y < image.h; ++y) {
    src_ptr = &data[(y * image.w) - xr];
    tmp_ptr = &tmp_data[y * image.w];

    for (int x = xr; x < xmax; ++x) {
      tmp_ptr[x] = .0;
      for (int dx = 0; dx < xw; ++dx)
	tmp_ptr[x] += ((matrix_type)src_ptr[x + dx]) * h_matrix[dx];
    }
  }

  // transform the horizontal convolution strip with v_matrix, leaving out all borders
  for (int x = xr; x < xmax; ++x) {
    src_ptr = &data[x + (yr * image.w)];
    tmp_ptr = &tmp_data[x];

    int offs = 0;
    for (int y = yr; y < ymax; ++y) {
      int doffs = offs;
      matrix_type sum = src_add * ((matrix_type)src_ptr[offs]);
      for (int dy = 0; dy < yw; ++dy) {
	sum += tmp_ptr[doffs] * v_matrix[dy];
	doffs += image.w;
      }
      uint8_t z = (uint8_t)(sum > 255 ? 255 : sum < 0 ? 0 : sum);
      src_ptr[offs] = z;
      offs += image.w;
    }
  }

  image.setRawData(); // invalidate as altered
  free (tmp_data);
}
