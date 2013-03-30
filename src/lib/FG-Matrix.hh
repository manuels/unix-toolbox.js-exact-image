/*
 * Copyright (C) 2007 - 2008 Valentin Ziegler, ExactCODE GmbH Germany.
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

#ifndef FG_MATRIX_HH__
#define FG_MATRIX_HH__

#include "Image.hh"
#include "DataMatrix.hh"

class FGMatrix : public DataMatrix<bool>
{
public:
  FGMatrix(Image& image, unsigned int fg_threshold);
  FGMatrix(const FGMatrix& source);
  FGMatrix(const FGMatrix& source, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
  ~FGMatrix();
};
  
#endif
