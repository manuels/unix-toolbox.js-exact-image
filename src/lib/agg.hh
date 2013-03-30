/*
 * Agg to ExactImage bridge.
 * Copyright (C) 2007 - 2012 René Rebe, ExactCODE GmbH, Germany
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

#include <agg_pixfmt_rgb.h>

#include <agg_rendering_buffer.h>
#include <agg_renderer_primitives.h>
#include <agg_renderer_scanline.h>
#include <agg_renderer_outline_aa.h>

#include <agg_scanline_p.h>
#include <agg_rasterizer_outline.h>
#include <agg_rasterizer_outline_aa.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_outline_aa.h>

#include <agg_gsv_text.h>

#include "Image.hh"

using agg::cover_type;
using agg::rect_i;

/* ExactImage to Agg::renderer_* bridge. René Rebe.
 */
class renderer_exact_image
{
public:
  
  typedef agg::pixfmt_rgb24 pixfmt_type;
  typedef pixfmt_type::color_type color_type;
  typedef pixfmt_type::row_data row_data;
  
  class blender_exact_image {
  public:
    typedef color_type::value_type value_type;
    typedef color_type::calc_type calc_type;
    
    enum base_scale_e 
      { 
	base_shift = color_type::base_shift,
	base_mask  = color_type::base_mask
      };
    
    static inline void blend_pix(Image::iterator& it, 
				 unsigned cr, unsigned cg, unsigned cb,
				 unsigned alpha, 
				 unsigned cover=0)
    {
      uint16_t r = 0, g = 0, b = 0, a = 0;
      *it; it.getRGBA (&r, &g, &b, &a);
      
      r = (value_type)(((cr - r) * alpha + (r << base_shift)) >> base_shift);
      g = (value_type)(((cg - g) * alpha + (g << base_shift)) >> base_shift);
      b = (value_type)(((cb - b) * alpha + (b << base_shift)) >> base_shift);
      a = (value_type)((alpha + a) - ((alpha * a + base_mask) >> base_shift));
      
      it.setRGBA (r, g, b, a);
      it.set (it);
      //p[Order::A] = (value_type)((alpha + a) - ((alpha * a + base_mask) >> base_shift));
    }
  };
  
  typedef blender_exact_image blender_type;
  
  //--------------------------------------------------------------------
  renderer_exact_image()
    : m_img (0), m_clip_box(1, 1, 0, 0)
  {}

  explicit renderer_exact_image(Image& img)
    : m_img (&img),
      m_clip_box (0, 0, m_img->w - 1, m_img->h - 1)
  {}
  
  void attach (Image& img)
  {
    m_img = &img;
    m_clip_box = rect_i (0, 0, m_img->w - 1, m_img->h - 1);
  }
 
  //--------------------------------------------------------------------
  // const pixfmt_type& ren() const;
  // pixfmt_type& ren();
  
  //--------------------------------------------------------------------
  unsigned width()  const { return m_img->w; }
  unsigned height() const { return m_img->h; }
  
  //--------------------------------------------------------------------
  bool clip_box(int x1, int y1, int x2, int y2);
  
  //--------------------------------------------------------------------
  void reset_clipping(bool visibility);
  
  //--------------------------------------------------------------------
  void clip_box_naked(int x1, int y1, int x2, int y2);

  //--------------------------------------------------------------------
  bool inbox(int x, int y) const;

  //--------------------------------------------------------------------
  const rect_i& clip_box() const { return m_clip_box;    }
  int           xmin()     const { return m_clip_box.x1; }
  int           ymin()     const { return m_clip_box.y1; }
  int           xmax()     const { return m_clip_box.x2; }
  int           ymax()     const { return m_clip_box.y2; }

  //--------------------------------------------------------------------
  const rect_i& bounding_clip_box();
  int           bounding_xmin();
  int           bounding_ymin();
  int           bounding_xmax();
  int           bounding_ymax();

  //--------------------------------------------------------------------
  void clear(const color_type& c)
  {
    for (Image::iterator it = m_img->begin(), it_end = m_img->end();
	 it != it_end; ++it) {
      it.setRGB ((uint16_t)c.r, (uint16_t)c.g, (uint16_t)c.b);
      it.set(it);
    }
  }
  
  //--------------------------------------------------------------------
  void copy_pixel(int x, int y, const color_type& c);

  //--------------------------------------------------------------------
  void blend_pixel(int x, int y, const color_type& c, cover_type cover)
  {
    // used in solid, primitive lines
    Image::iterator it = m_img->begin();
    it = it.at (x, y);
    
    blender_type::blend_pix(it, c.r, c.g, c.b, c.a, cover);
  }

  //--------------------------------------------------------------------
  color_type pixel(int x, int y) const;

  //--------------------------------------------------------------------
  void copy_hline(int x1, int y, int x2, const color_type& c);

  //--------------------------------------------------------------------
  void copy_vline(int x, int y1, int y2, const color_type& c);
  
  //--------------------------------------------------------------------
  void blend_hline(int x1, int y, int x2, 
		   const color_type& c, cover_type cover)
  {
    if(x1 > x2) { int t = x2; x2 = x1; x1 = t; }
    if(y  > ymax()) return;
    if(y  < ymin()) return;
    if(x1 > xmax()) return;
    if(x2 < xmin()) return;
    
    if(x1 < xmin()) x1 = xmin();
    if(x2 > xmax()) x2 = xmax();
    
    // m_ren->blend_hline(x1, y, x2 - x1 + 1, c, cover);
    int len = x2 - x1 + 1;
    if (c.a)
      {
	typedef color_type::calc_type calc_type;
	
	Image::iterator it = m_img->begin();
	it = it.at (x1, y);
	
	calc_type alpha = (calc_type(c.a) * (cover + 1)) >> 8;
	if(alpha == color_type::base_mask)
	  {
	    // ((value_type*)&v)[order_type::A] = c.a;
	    it.setRGBA ((uint16_t)c.r, (uint16_t)c.g, (uint16_t)c.b, (uint16_t)c.a);
	    do
	      {
		it.set(it);
		++it; // Note: assumes to keep the RGB value!
	      }
	    while(--len);
	  }
	else
	  {
	    if(cover == 255)
	      {
		do
		  {
		    blender_type::blend_pix(it, c.r, c.g, c.b, alpha);
		    ++it;
		  }
		while(--len);
	      }
	    else
	      {
		do
		  {
		    blender_type::blend_pix(it, c.r, c.g, c.b, alpha, cover);
		    ++it;
		  }
		while(--len);
	      }
	  }
      }
  }

  //--------------------------------------------------------------------
  void blend_vline(int x, int y1, int y2, 
		   const color_type& c, cover_type cover);
  
  //--------------------------------------------------------------------
  void copy_bar(int x1, int y1, int x2, int y2, const color_type& c);

  //--------------------------------------------------------------------
  void blend_bar(int x1, int y1, int x2, int y2, 
		 const color_type& c, cover_type cover);

  //--------------------------------------------------------------------
  void blend_solid_hspan(int x, int y, int len, 
			 const color_type& c, 
			 const cover_type* covers)
  {
    if(y > ymax()) return;
    if(y < ymin()) return;
    
    if(x < xmin())
      {
	len -= xmin() - x;
	if(len <= 0) return;
	covers += xmin() - x;
	x = xmin();
      }
    if(x + len > xmax())
      {
	len = xmax() - x + 1;
	if(len <= 0) return;
      }
    
    // m_ren->blend_solid_hspan(x, y, len, c, covers);
    if (c.a)
      {
	typedef color_type::calc_type calc_type;
	
	Image::iterator it = m_img->begin();
	it = it.at (x, y);
	do 
	  {
	    calc_type alpha = (calc_type(c.a) * (calc_type(*covers) + 1)) >> 8;
	    if(alpha == color_type::base_mask)
	      {
		it.setRGBA ((uint16_t)c.r, (uint16_t)c.g, (uint16_t)c.b, color_type::base_mask);
		it.set(it);
	      }
	    else
	      {
		blender_type::blend_pix(it, c.r, c.g, c.b, alpha, *covers);
	      }
	    ++it;
	    ++covers;
	  }
	while(--len);
      }
  }
  
  //--------------------------------------------------------------------
  void blend_solid_vspan(int x, int y, int len, 
			 const color_type& c, 
			 const cover_type* covers)
  {
    if(x > xmax()) return;
    if(x < xmin()) return;
    
    if(y < ymin())
      {
	len -= ymin() - y;
	if(len <= 0) return;
	covers += ymin() - y;
	y = ymin();
      }
    if(y + len > ymax())
      {
	len = ymax() - y + 1;
	if(len <= 0) return;
      }

    // m_ren->blend_solid_vspan(x, y, len, c, covers);
    if (c.a)
      {
	typedef color_type::calc_type calc_type;
	
	Image::iterator it = m_img->begin();
	
	do 
	  {
	    it = it.at (x, y);
	    calc_type alpha = (calc_type(c.a) * (calc_type(*covers) + 1)) >> 8;
	    if(alpha == color_type::base_mask)
	      {
		it.setRGBA ((uint16_t)c.r, (uint16_t)c.g, (uint16_t)c.b, color_type::base_mask);
		it.set(it);
	      }
	    else
	      {
		blender_type::blend_pix(it, c.r, c.g, c.b, alpha, *covers);
	      }
	    ++y;
	    ++covers;
	  }
	while(--len);
      }
  }
  
  //--------------------------------------------------------------------
  void copy_color_hspan(int x, int y, int len, const color_type* colors);
  
  //--------------------------------------------------------------------
  void copy_color_vspan(int x, int y, int len, const color_type* colors);

  //--------------------------------------------------------------------
  void blend_color_hspan(int x, int y, int len, 
			 const color_type* colors, 
			 const cover_type* covers,
			 cover_type cover = agg::cover_full);
  
  //--------------------------------------------------------------------
  void blend_color_vspan(int x, int y, int len, 
			 const color_type* colors, 
			 const cover_type* covers,
			 cover_type cover = agg::cover_full);

  //--------------------------------------------------------------------
  rect_i clip_rect_area(rect_i& dst, rect_i& src, int wsrc, int hsrc) const;

  //--------------------------------------------------------------------
  template<class RenBuf>
  void copy_from(const RenBuf& src, 
		 const rect_i* rect_src_ptr = 0, 
		 int dx = 0, 
		 int dy = 0);
  
  //--------------------------------------------------------------------
  template<class SrcPixelFormatRenderer>
  void blend_from(const SrcPixelFormatRenderer& src, 
		  const rect_i* rect_src_ptr = 0, 
		  int dx = 0, 
		  int dy = 0,
		  cover_type cover = agg::cover_full);

  //--------------------------------------------------------------------
  template<class SrcPixelFormatRenderer>
  void blend_from_color(const SrcPixelFormatRenderer& src, 
			const color_type& color,
			const rect_i* rect_src_ptr = 0, 
			int dx = 0, 
			int dy = 0,
			cover_type cover = agg::cover_full);

  //--------------------------------------------------------------------
  template<class SrcPixelFormatRenderer>
  void blend_from_lut(const SrcPixelFormatRenderer& src, 
		      const color_type* color_lut,
		      const rect_i* rect_src_ptr = 0, 
		      int dx = 0, 
		      int dy = 0,
		      cover_type cover = agg::cover_full);

private:
  
  Image* m_img;
  rect_i m_clip_box;
};


typedef agg::pixfmt_rgb24 pixfmt;

typedef renderer_exact_image renderer_base;

/* render vector data */
typedef agg::renderer_primitives<renderer_base> renderer_prim;
typedef agg::renderer_outline_aa<renderer_base> renderer_oaa;

typedef agg::renderer_scanline_bin_solid<renderer_base> renderer_bin;
typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_aa;


/* raster into final buffer */
typedef agg::scanline_p8 scanline;

typedef agg::rasterizer_outline<renderer_prim> rasterizer_outline;
typedef agg::rasterizer_outline_aa<renderer_oaa> rasterizer_oaa;
typedef agg::rasterizer_scanline_aa<> rasterizer_scanline;

//typedef agg::pattern_filter_bilinear_rgba8 pattern_filter;
//typedef agg::line_image_pattern_pow2<pattern_filter> image_pattern;
//typedef agg::renderer_outline_image<renderer_base, image_pattern> renderer_img;
//typedef agg::rasterizer_outline_aa<renderer_img> rasterizer_oimg;
