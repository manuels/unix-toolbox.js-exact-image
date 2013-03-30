/*
 * Image Segmentation
 * Copyright (C) 2007 Valentin Ziegler and René Rebe
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


#include <vector>
#include "FG-Matrix.hh"

class Segment
{
public:
  unsigned int x;
  unsigned int y;
  unsigned int w;
  unsigned int h;
  Segment* parent;
  std::vector <Segment*> children;

  Segment(unsigned int ix, unsigned int iy, unsigned int iw, unsigned int ih, Segment* iparent=0);
  ~Segment();

  bool Subdivide(const FGMatrix& img, double tolerance, unsigned int min_length, bool horizontal);

  // Draws a (red) frame around the segment
  void Draw(Image& output, uint16_t r = 255, uint16_t g = 0, uint16_t b = 0);

private:

  void InsertChild(unsigned int start, unsigned int end, bool horizontal);

  // count foreground pixels in horizontal/vertical lines
  unsigned int* Count(const FGMatrix& img, bool horizontal);
};



// returns a segmentation of foreground matrix <img>.
// <tolerance> is the maximum fraction of foreground pixels allowed in a separator line
// <min_w> and <min_h> denote the minimum separator width and height
Segment* segment_image(const FGMatrix& img, double tolerance, unsigned int min_w, unsigned int min_h);

