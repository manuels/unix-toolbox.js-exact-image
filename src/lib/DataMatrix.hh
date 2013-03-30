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

#ifndef DATA_MATRIX_HH__
#define DATA_MATRIX_HH__

template <typename T>
class DataMatrix
{
public:
  DataMatrix(unsigned int iw, unsigned int ih)
  {
    w=iw;
    h=ih;
    master=true;

    data=new T*[w];
    for (unsigned int x=0; x<w; x++)
      data[x]=new T[h];
  }

  DataMatrix(const DataMatrix<T>& source, unsigned int ix, unsigned int iy, unsigned int iw, unsigned int ih)
  {
    w=iw;
    h=ih;
    master=false;

    data=new T*[w];
    for (unsigned int x=0; x<w; x++)
      data[x]=source.data[x+ix]+iy;
  }
  
  virtual ~DataMatrix()
  {
    if (master)
      for (unsigned int x=0; x<w; x++)
	delete[] data[x];
    delete[] data;
  }

  unsigned int w;
  unsigned int h;
  T** data;
  bool master;

  const T& operator() (unsigned int x, unsigned int y) const
  {
    return data[x][y];
  }

  T& operator() (unsigned int x, unsigned int y)
  {
    return data[x][y];
  }
};

#endif
  
