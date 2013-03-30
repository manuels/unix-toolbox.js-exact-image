/*
 * Copyright (C) 2008-2010 René Rebe
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

/*
 * Based on the svg_test application window,
 * Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
 */

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"

#include "agg_svg_parser.hh"

#include "svg.hh"

#include "agg.hh" // EI Agg

int SVGCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
  agg::svg::path_renderer m_path;
  agg::svg::parser p (m_path);
  
  if (stream->peek () != '<')
    return false;

  try
    {
      p.parse(*stream);
    }
  catch(agg::svg::exception& e)
    {
      std::cerr << e.msg() << std::endl;
      return false;
    }
  
  double min_x = 0;
  double min_y = 0;
  double max_x = 0;
  double max_y = 0;
  
  const double expand = 0;
  const double gamma = 1;
  
  m_path.arrange_orientations();
  m_path.bounding_rect (&min_x, &min_y, &max_x, &max_y);

  // prevent crash for invalid constraints
  if (min_x > max_x)
    { double t = max_x; max_x = min_x; min_x = t; }
  else if (min_x == max_x)
    max_x += 1;
  if (min_y > max_y)
    { double t = max_y; max_y = min_y; min_y = t; }
  else if (min_y == max_y)
    max_y += 1;

  image.bps = 8; image.spp = 3;
  image.resize ((int)(max_x - min_x), (int)(max_y - min_y));
  
  renderer_exact_image rb (image);
  typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
  renderer_solid ren (rb);
  
  rb.clear (agg::rgba(1,1,1));
  
  agg::rasterizer_scanline_aa<> ras;
  agg::scanline_p8 sl;
  agg::trans_affine mtx;
  
  ras.gamma(agg::gamma_power(gamma));
  mtx *= agg::trans_affine_translation ((min_x + max_x) * -0.5,
					(min_y + max_y) * -0.5);
  //mtx *= agg::trans_affine_scaling (scale);
  mtx *= agg::trans_affine_translation ((min_x + max_x) * 0.5,
					(min_y + max_y) * 0.5);
  
  m_path.expand(expand);
  m_path.render(ras, sl, ren, mtx, rb.clip_box(), 1.0);
  
  //std::cerr << "Vertices=" << m_path.vertex_count() << " Time=" << tm << " ms" std::endl;
  
  return true;
}

bool SVGCodec::writeImage (std::ostream* stream, Image& image, int quality,
                           const std::string& compress)
{
  return false;
}

SVGCodec svg_loader;
