/*
 * Vector element rasterization, via Agg.
 * Copyright (C) 2007 - 2011 Rene Rebe, ExactCODE GmbH
 * Copyright (C) 2007 Susanne Klaus, ExactCODE GmbH
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

#ifndef VECTORIAL_HH
#define VECTORIAL_HH

#include "Image.hh"
#include <vector>

#include "config.h"

#include "agg_math_stroke.h"
#include "agg_path_storage.h"

#include "agg_trans_affine.h"

class Path
{
public:
  Path ();
  ~Path ();

  void moveTo (double x, double y);
  void addLine (double x, double y);
  void addLineTo (double x, double y);
  void addRect (double x, double y, double x2, double y2);
  
  // the first is relative, the second to an an absolute 2nd point
  void addArc (double rx, double ry,  double angle,
	       double dx, double dy);
  void addArcTo (double rx, double ry,  double angle,
		 double x2, double y2);
  
  /* TODO:
     - addEllipse
  */
    
  void addCurveTo (double, double, double, double);
  // or explicitly differentiate by naming this one QuadCurve ???
  void addCurveTo (double, double, double, double, double, double);
  
  void end ();
  void close ();
  void clear (); // path data, not the style
  
  void setFillColor (double r, double g, double b, double a = 1.0);
  void setLineWidth (double width);
  void setLineDash (double offset, const std::vector<double>& dashes);
  void setLineDash (double offset, const double* dashes, int n);
  
  typedef agg::line_cap_e line_cap_t;
  void setLineCap (line_cap_t cap);
  
  typedef agg::line_join_e line_join_t;
  void setLineJoin (line_join_t join);
  
  /* TODO:
     - clip
     - control anti-aliasing
     (- gradients)
  */
  
  enum filling_rule_t
    {
      fill_non_zero = agg::fill_non_zero,
      fill_even_odd = agg::fill_even_odd,
      fill_none = 0xff
    };

  void draw (Image& image, filling_rule_t fill = fill_none);
  
  // TODO: sophisticated text API, including font to use,
  // kerning, hinting, transform, etc.
  // void addText (char* text, double height);
 
  // temp. simple text draw method
  bool drawText (Image& image, const char* text, double height,
		 const char* fontfile = 0, agg::trans_affine mtx = agg::trans_affine(),
		 filling_rule_t fill = fill_non_zero,
		 double* w = 0, double* h = 0, double* dx = 0, double* dy = 0);
  bool drawTextOnPath (Image& image, const char* text, double height,
		       const char* fontfile = 0); // TODO: , filling_rule_t fill = fill_non_zero);
  
protected:
  agg::path_storage path;
  
  // quick hack ("for now")
  double r, g, b, a, line_width, dashes_start_offset;
  std::vector <double> dashes;
  
  line_cap_t line_cap;
  line_join_t line_join;
};

#endif
