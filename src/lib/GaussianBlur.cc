/*
 * Gaussian Blur.
 * Copyright (C) 2008, Valentin Ziegler
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
 */

#include "Matrix.hh"
#include "GaussianBlur.hh"

void GaussianBlur(Image& image, double standard_deviation, int radius)
{
  double sd = standard_deviation;
  if (radius <= 0) {
    double thresh=1.0/255.0;
    double divisor=0.0;
    radius=-1;
    double factor;
    do {
      radius++;
      factor=exp (-((float)radius*radius) / (2.*sd*sd));
      divisor+=(radius==0) ? factor : 2.0*factor;
    } while (factor/(divisor*divisor) > thresh);
  }

  // compute kernel (convolution matrix to move over the iamge)
  matrix_type divisor = 0;
    
  matrix_type matrix[radius+1];

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
  }
    
  decomposable_sym_convolution_matrix (image, matrix, matrix, radius, radius, 0.0);
}
