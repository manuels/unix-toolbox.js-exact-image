/*
 * Convolution Matrix.
 * Copyright (C) 2006, 2010 René Rebe
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
 */

#ifndef MATRIX_HH
#define MATRIX_HH

#include "Image.hh"

// any matrix and devisior
// on my AMD Turion speed is: double > int > float
// and in 32bit mode: double > float > int ?
typedef double matrix_type;

void convolution_matrix (Image& image, const matrix_type* matrix, int xw, int yw,
			 matrix_type divisor);

// convolution matrix code if matrix[i][j] is decomposable to h_matrix[i]*v_matrix[j]
// the original image is multiplied with src_add and added to the result.
void decomposable_convolution_matrix (Image& image, const matrix_type* h_matrix, const matrix_type* v_matrix, int xw, int yw,
				      matrix_type src_add);


// h_matrix contains entrys m[0]...m[xw]. It is assumed, that m[-i]=m[i]. Same for v_matrix.
void decomposable_sym_convolution_matrix (Image& image, const matrix_type* h_matrix, const matrix_type* v_matrix, int xw, int yw,
					  matrix_type src_add);

#endif
