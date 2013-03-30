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

#include "FG-Matrix.hh"

FGMatrix::FGMatrix(Image& image, unsigned int fg_threshold)
  : DataMatrix<bool>(image.w, image.h)
{
  unsigned int line=0;
  unsigned int row=0;
  Image::iterator i=image.begin();
  Image::iterator end=image.end();
  for (; i!=end ; ++i) {
    data[row][line]=((*i).getL() < fg_threshold);

    if (++row == (unsigned int)image.w) {
      line++;
      row=0;
    }
  }
}

FGMatrix::FGMatrix(const FGMatrix& source)
  : DataMatrix<bool>(source, 0, 0, source.w, source.h)
{
}

FGMatrix::FGMatrix(const FGMatrix& source, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
  : DataMatrix<bool>(source, x,y,w,h)
{
}
  
FGMatrix::~FGMatrix()
{
}
