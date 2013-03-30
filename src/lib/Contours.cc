/*
 * Copyright (C) 2007 Valentin Ziegler, ExactCODE GmbH Germany.
 *               2008 - 2009 Rene Rebe, ExactCODE GmbH Germany.
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

#include "Contours.hh"

/*
 list of pixel traversals (clockwise order):
 0: on left side upwards
 1: on pixel top to the right
 2: on right side downwards
 3: on pixel bottom to the left
*/

// bitmask for pixel traversal - first bit is there to indicate foreground pixel
const unsigned int pixelborder[4]={2,4,8,16};

struct StartCheck {
  int dx;
  int dy;
};

const StartCheck startchecks[4]={
  {-1,0}, {0,-1}, {1,0}, {0,1}
};

struct Transition {
  int dx;
  int dy;
  unsigned int border;
};

const Transition transitions[4][3]={
  { {-1, -1, 3}, { 0, -1, 0}, {0, 0, 1} },
  { { 1, -1, 0}, { 1,  0, 1}, {0, 0, 2} },
  { { 1,  1, 1}, { 0,  1, 2}, {0, 0, 3} },
  { {-1,  1, 2}, {-1,  0, 3}, {0, 0, 0} }
};


inline bool Step(Contours::VisitMap& map, int& x, int& y, int& border)
{
  for (unsigned int i=0; i<3; i++) {
    const Transition& t=transitions[border][i];
    const int xx=x+t.dx;
    const int yy=y+t.dy;
    // do we have a foreground pixel ?
    if (xx >= 0 && xx < (signed int)map.w && yy >= 0 && yy < (signed int)map.h && map(xx,yy) > 0) {
      if ((map(xx,yy) & pixelborder[t.border]) == 0) { // go there
	x=xx;
	y=yy;
	border=t.border;
	map(x,y) |= pixelborder[border];
	return true;
      } else // already been there before
	return false;
    }
  }

  // note: this code line is never reached, because last transition is always {0,0,b}
  return false;
}

inline bool Start(Contours::VisitMap& map, int x, int y, int border)
{
  if ((map(x,y) & pixelborder[border]) == 0 ) {
    const StartCheck& c=startchecks[border];
    const int xx=x+c.dx;
    const int yy=y+c.dy;
    // do we have a background pixel ?
    if (xx < 0 || xx >= (signed int)map.w || yy < 0 || yy >= (signed int)map.h || ((map(xx,yy) & 1) == 0)) {
      map(x,y)|=pixelborder[border];
      return true;
    }
  }
  return false;
}

Contours::Contours(const FGMatrix& image)
{
  VisitMap map(image.w, image.h);
  for (unsigned int x=0; x<map.w; x++)
    for (unsigned int y=0; y<map.h; y++)
      map(x,y)=(image(x,y)) ? 1 : 0;

  for (unsigned int x=0; x<map.w; x++)
    for (unsigned int y=0; y<map.h; y++)
      if (map(x,y) > 0)
	for (unsigned int border=0; border < 4; border++)
	  if (Start(map,x,y,border)) {
	    int xx=x;
	    int yy=y;
	    int bborder=border;
	    Contour* current=new Contour();
	    contours.push_back(current);
	    do {
	      current->push_back(std::pair<unsigned int, unsigned int>(xx, yy));
	    } while (Step(map, xx, yy, bborder));
	  }
}

Contours::~Contours()
{
  for (unsigned int i=0; i<contours.size(); i++)
    delete contours[i];
}



MidContours::MidContours(const FGMatrix& image)
{
  Contour* current = new Contour();
  contours.push_back (current);
  
  // thru the whole "image" in x-direction
  for (unsigned int y = 0; y < image.h; y++)
    for (unsigned int x = 0; x < image.w; x++)
      {
	// something?
	if (image(x,y))
	  {
	      // search end of region of scanline
	    const unsigned int x1 = x++;
	    while (x < image.w && image(x,y))
	      x++;
	    
	    // region [x1,x], midpoint mx:
	    const unsigned int mx = (x1 + x) / 2;
	    
	    current->push_back(std::pair<unsigned int, unsigned int>(mx, y));
	  }
      }
  
  // thru the whole "image" in x-direction
  for (unsigned int x = 0; x < image.w; x++)
    for (unsigned int y = 0; y < image.h; y++)
      {
	// something?
	if (image(x,y))
	  {
	    // search end of region of scanline
	    const unsigned int y1 = y++;
	    while (y < image.h && image(x,y))
	      y++;
	    
	    // region [y1,y], midpoint mx:
	    const unsigned int my = (y1 + y) / 2;
	      
	    current->push_back(std::pair<unsigned int, unsigned int>(x, my));
	  }
      }
  
  // TODO: filter duplicates and/or clean spots without neighbour
  // TODO: sub-pixel accuracy would help
}



InnerContours::InnerContours(const FGMatrix& image)
{
  VisitMap map(image.w, image.h);
  for (unsigned int x=0; x<map.w; x++)
    for (unsigned int y=0; y<map.h; y++)
      map(x,y) = 0;
  
  // distance to border
  for (unsigned int x=0; x<map.w; x++)
    for (unsigned int y=0; y<map.h; y++)
      if (image(x,y))
	{
	  int accu = 1;
	  
	  for (int r = 1; ; ++r)
	    {
	      int add =
		RecursiveDist(image, x, y, LEFT, r) +
		RecursiveDist(image, x, y, RIGHT, r) +
		RecursiveDist(image, x, y, UP, r) +
		RecursiveDist(image, x, y, DOWN, r);
	      accu += add;
	      if (add < 4)
		break;
	    }
	  map(x,y) = accu;
	}
  
  // only use local maxima
  VisitMap newmap(image.w, image.h);
  for (unsigned int x = 0; x < map.w; x++)
    for (unsigned int y = 0; y < map.h; y++)
      {
	newmap(x,y) = 0;
	int d = map(x,y);
	if (d == 0) continue;
	
	if (x > 0 && d < map(x-1, y))
	  continue;
	if (y > 0 && d < map(x, y-1))
	  continue;
	
	if (x+1 < map.w && d < map(x+1, y))
	  continue;
	if (y+1 < map.h && d < map(x, y+1))
	  continue;
	
	newmap(x,y) = 1;
      }
  
  for (unsigned int x = 0; x < map.w; x++)
    for (unsigned int y = 0; y < map.h; y++)
      {
	if (newmap (x,y)) {
	  Contour* current = new Contour();
	  contours.push_back(current);
	  RecursiveTrace (newmap, current, x, y);
	}
      }
}

bool InnerContours::RecursiveTrace(VisitMap& newmap, Contour* current,
				   unsigned int x, unsigned int y)
{
  if (newmap(x,y)) {
    newmap(x,y) = 0;
    current->push_back(std::pair<unsigned int, unsigned int>(x, y));
    
    int
      x1 = x > 0 ? x-1 : 0,
      y1 = y > 0 ? y-1 : 0,
      x2 = x+1 < newmap.w ? x+1 : x,
      y2 = y+1 < newmap.h ? y+1 : y;
    
    if (RecursiveTrace (newmap, current, x, y2))
      return true;
    if (RecursiveTrace (newmap, current, x1, y2))
      return true;
    if (RecursiveTrace (newmap, current, x2, y2))
      return true;
    
    if (RecursiveTrace (newmap, current, x2, y))
      return true;
    if (RecursiveTrace (newmap, current, x2, y1))
      return true;
    if (RecursiveTrace (newmap, current, x, y1))
      return true;

    if (RecursiveTrace (newmap, current, x1, y1))
      return true;
    if (RecursiveTrace (newmap, current, x1, y))
      return true;
    
    
    return true;
  }
  return false;
}

unsigned int InnerContours::RecursiveDist(const FGMatrix& image,
					  int x, int y,
					  dir d, unsigned int r)
{
  switch (d) {
  case LEFT:
    x -= r;
    if (x < 0)
      return 0;
    break;
  case UP:
    y -= r;
    if (y < 0)
      return 0;
    break;
  case RIGHT:
    x += r;
    if (x >= (int)image.w)
      return 0;
    break;
  case DOWN:
    y += r;
    if (y >= (int)image.h)
      return 0;
    break;
  }
  
  return image(x,y) ? 1 : 0;
}
