/*
 * The ExactImage library's convert compatible command line frontend.
 * Copyright (C) 2005 - 2011 René Rebe, ExactCODE GmbH
 * Copyright (C) 2005, 2008 Archivista GmbH
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

#include <math.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <algorithm>
#include <iterator>

#include <limits>

#include <list>

#include "config.h"

#include "ArgumentList.hh"

#include "Image.hh"
#include "Codecs.hh"

#include "Colorspace.hh"

#include "low-level.hh"

#include "scale.hh"
#include "crop.hh"
#include "rotate.hh"
#include "canvas.hh"

#include "Matrix.hh"

#include "riemersma.h"
#include "floyd-steinberg.h"

#include "vectorial.hh"
#include "GaussianBlur.hh"

#include "agg_trans_affine.h"

/* Let's reuse some parts of the official, stable API to avoid
 * duplicating code.
 *
 * This also includes the foreground/background color and vector
 * drawing style.
 */

#include "api/api.cc"

#include "C.h"

#include <functional>

using namespace Utility;

// the global "stack" of images we work on
std::list<Image*> images;
typedef std::list<Image*>::iterator images_iterator;

#define FOR_ALL_IMAGES(f,...) \
  for (images_iterator it = images.begin(); it != images.end(); ++it) \
    f(**it, ##__VA_ARGS__)

static void freeImages()
{
  while (!images.empty()) {
    delete(images.back());
    images.pop_back();
  }
}

Argument<int> arg_quality ("", "quality",
			   "quality setting used for writing compressed images\n\t\t"
			   "integer range 0-100, the default is 75",
			   0, 1, true, true);

Argument<std::string> arg_compression ("", "compress",
				       "compression method for writing images e.g. G3, G4, Zip, ...\n\t\t"
				       "depending on the output format, a reasonable setting by default",
				       0, 1, true, true);

Argument<std::string> arg_decompression ("", "decompress",
					 "decompression method for reading images e.g. thumb\n\t\t"
					 "depending on the input format, allowing to read partial data",
					 0, 1, true, true);


Argument<double> arg_stroke_width ("", "stroke-width",
				   "the stroke width for vector primitives",
				   0, 1, true, true);

#if WITHFREETYPE == 1

Argument<double> arg_text_rotation ("", "text-rotation",
				    "draw text using specified rotation",
				    0, 1, true, true);

Argument<std::string> arg_font ("", "font",
				"draw text using specified font file",
				0, 1, true, true);
#endif

bool convert_input (const Argument<std::string>& arg)
{
  Image* image = 0;

  // save an empty template if we got one (for loading RAW images)
  if (!images.empty() && images.front()->getRawData() == 0) {
    image = images.front();
    images.pop_front();
  }
  freeImages();
  
  for (int j = 0; j < arg.Size(); ++j)
    {
      std::string file = arg.Get(j);
      std::string cod = ImageCodec::getCodec(file);
      std::string ext = ImageCodec::getExtension(file);

      std::ifstream stream(file.c_str(), std::ios::in | std::ios::binary);
      
      std::string decompression = "";
      if (arg_decompression.Size())
	decompression = arg_decompression.Get();
      
      for (int i = 0, n = 1; i < n; ++i)
	{
	  if (!image)
	    image = new Image;

	  int ret = ImageCodec::Read(&stream, *image, cod, decompression, i);
	  if (ret <= 0) {
	    std::cerr << "Error reading input file " << arg.Get(j) << ", image: " << i << std::endl;
	    delete image;
	    return false;
	  }
	  if (i == 0)
	    n = ret;
	  
	  images.push_back(image);
	  image = 0;
	}
    }

  if (image) delete image;
  
  return true;
}

bool convert_append (const Argument<std::string>& arg)
{
  Image* base = 0;
  for (images_iterator it = images.begin(); it != images.end(); ++it) {
    if (it == images.begin())
      base = *it;
    else
      append(*base, **it);
  }
  return true;
}

bool convert_output (const Argument<std::string>& arg)
{
  int quality = 75;
  if (arg_quality.Size())
    quality = arg_quality.Get();
  std::string compression = "";
  if (arg_compression.Size())
    compression = arg_compression.Get();
  
  int i = 0, f = 0;
  images_iterator it = images.begin();
  ImageCodec* codec = 0;
  std::fstream* stream = 0;

  for (; it != images.end(); ++it)
    {
      if (!codec)
	{
	  if (f >= arg.Size())
	    break;
	  
	  i = 0;
	  // no multi-page codec, yet, try to create one
	  std::string file = arg.Get(f);
	  std::string cod = ImageCodec::getCodec(file);
	  std::string ext = ImageCodec::getExtension(file);
	  stream = new std::fstream(file.c_str(),
				    std::ios::in | std::ios::out | std::ios::trunc);
	  
	  codec = ImageCodec::MultiWrite(stream, cod, ext);
	  // if we got no codec, write a classic, single-page file
	  if (!codec) {
	    if (!ImageCodec::Write(stream, **it, cod, ext, quality, compression))
	      std::cerr << "Error writing output file, image " << i << std::endl;
	    delete stream; stream = 0;
	  }
	  ++f; // filename used, next
	}
      
      // do we (now) have a codec?, write multi-page image
      if (codec)
	{
	  if (!codec->Write(**it, quality, compression, i))
	    std::cerr << "Error writing output file, image " << i << std::endl;
	  ++i;
	}
    }
  
  // if we had a multi-page codec and stream free them now
  if (codec)
    delete codec;
  if (stream)
    delete stream;
  
  if (it != images.end())
    std::cerr << "Error: " << std::distance(it, images.end())
	      << " image(s) left for writing" << std::endl;
  if (f < arg.Size())
    std::cerr << "Error: " << arg.Size() - f
	      << " filename(s) left for writing" << std::endl;
  
  return true;
}

bool convert_split (const Argument<std::string>& arg)
{
  // TODO: we could split the result into the iamge stack
  if (images.empty() || (*images.begin())->getRawData() == 0) {
    std::cerr << "No image available." << std::endl;
    return false;
  }
  
  Image& image = **images.begin();
  
  Image split_image;
  split_image.copyMeta(**images.begin());
  
  split_image.h /= arg.count;
  if (split_image.h == 0) {
    std::cerr << "Resulting image size too small." << std::endl
	      << "The resulting slices must have at least a height of one pixel."
	      << std::endl;
    
    return false;
  }
  
  int quality = 70;
  if (arg_quality.Size())
    quality = arg_quality.Get();
  std::string compression = "";
  if (arg_compression.Size())
    compression = arg_compression.Get();

  int err = 0;
  for (int i = 0; i < arg.Size(); ++i)
    {
      std::cerr << "Writing file: " << arg.Get(i) << std::endl;
      split_image.setRawDataWithoutDelete
        (image.getRawData() + i * split_image.stride() * split_image.h);
      if (!ImageCodec::Write (arg.Get(i), split_image, quality, compression)) {
	err = 1;
	std::cerr << "Error writing output file." << std::endl;
      }
    }
  split_image.setRawDataWithoutDelete(0); // not deallocate in dtor
  
  return err == 0;
}

bool convert_colorspace (const Argument<std::string>& arg)
{
  FOR_ALL_IMAGES(colorspace_by_name, arg.Get().c_str());
  return true; // TODO return value
}

bool convert_normalize (const Argument<bool>& arg)
{
  FOR_ALL_IMAGES(normalize);
  return true;
}

bool convert_brightness (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(brightness_contrast_gamma, f, .0, 1.0);
  return true;
}

bool convert_contrast (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(brightness_contrast_gamma, .0, f, 1.0);
  return true;
}

bool convert_gamma (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(brightness_contrast_gamma, .0, .0, f);
  return true;
}

bool convert_blur (const Argument<double>& arg)
{
  double standard_deviation = arg.Get();
  FOR_ALL_IMAGES(GaussianBlur, standard_deviation);
  return true;
}

bool convert_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(scale, f, f);
  return true;
}

bool convert_thumbnail_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(thumbnail_scale, f, f);
  return true;
}

bool convert_hue (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(hue_saturation_lightness, f, 0, 0);
  return true;
}

bool convert_saturation (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(hue_saturation_lightness, 0, f, 0);
  return true;
}

bool convert_lightness (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(hue_saturation_lightness, 0, 0, f);
  return true;
}

bool convert_nearest_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(nearest_scale, f, f);
  return true;
}

bool convert_bilinear_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(bilinear_scale, f, f);
  return true;
}

bool convert_bicubic_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(bicubic_scale, f, f);
  return true;
}

bool convert_box_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(box_scale, f, f);
  return true;
}

bool convert_ddt_scale (const Argument<double>& arg)
{
  double f = arg.Get();
  FOR_ALL_IMAGES(ddt_scale, f, f);
  return true;
}

bool convert_flip (const Argument<bool>& arg)
{
  FOR_ALL_IMAGES(flipY);
  return true;
}

bool convert_flop (const Argument<bool>& arg)
{
  FOR_ALL_IMAGES(flipX);
  return true;
}

bool convert_rotate (const Argument<double>& arg)
{
  FOR_ALL_IMAGES(rotate, arg.Get(), background_color);
  return true;
}

bool convert_convolve (const Argument<double>& arg)
{
  double divisor = 0;
  const std::vector<double>& v = arg.Values ();
  int n = (int)sqrt(v.size());
  
  for (unsigned int i = 0; i < v.size(); ++i)
    divisor += v[i];
  
  if (divisor == 0)
    divisor = 1;
  
  FOR_ALL_IMAGES(convolution_matrix, &v[0], n, n, divisor);
  return true;
}

bool convert_dither_floyd_steinberg (const Argument<int>& arg)
{
  bool ret = true;
  for (images_iterator it = images.begin(); it != images.end(); ++it)
    {
      if ((*it)->bps != 8) {
	std::cerr << "Can only dither 8 bit data right now." << std::endl;
	ret = false;
      }
      else
	FloydSteinberg((*it)->getRawData(), (*it)->w, (*it)->h, arg.Get(), (*it)->spp);
    }
  return ret;
}

bool convert_dither_riemersma (const Argument<int>& arg)
{
  bool ret = true;
  for (images_iterator it = images.begin(); it != images.end(); ++it)
    {
      if ((*it)->bps != 8) {
	std::cerr << "Can only dither 8 bit data right now." << std::endl;
	ret = false;
      }
      else
	Riemersma ((*it)->getRawData(), (*it)->w, (*it)->h, arg.Get(), (*it)->spp);
    }
  return ret;
}

bool convert_edge (const Argument<bool>& arg)
{
  matrix_type matrix[] = { -1.0, 0.0,  1.0,
                           -2.0, 0.0,  2.0,
                           -1.0, 0.0, -1.0 };

  FOR_ALL_IMAGES(convolution_matrix, matrix, 3, 3, (matrix_type)3.0);
  return true;
}

bool convert_resolution (const Argument<std::string>& arg)
{
  int xres, yres, n;
  // parse
  
  // TODO: pretty C++ parser
  if ((n = sscanf(arg.Get().c_str(), "%dx%d", &xres, &yres)))
    {
      if (n < 2)
	yres = xres;
      for (images_iterator it = images.begin(); it != images.end(); ++it)
	(*it)->setResolution(xres, yres);
      return true;
    }
  std::cerr << "Resolution '" << arg.Get() << "' could not be parsed." << std::endl;
  return false;
}

bool convert_size (const Argument<std::string>& arg)
{
  int w, h, n;
  // parse
  
  // TODO: pretty C++ parser
  if ((n = sscanf(arg.Get().c_str(), "%dx%d", &w, &h)) == 2)
    {
      // this is mostly used to set the size for raw data loads
      // so we need to have at least one (empty) image
      if (images.empty())
	images.push_back(new Image);
      
      for (images_iterator it = images.begin(); it != images.end(); ++it)
	{
	  (*it)->w = w;
	  (*it)->h = h;
	  (*it)->setRawData(0);
	}
      
      return true;
    }
  std::cerr << "Size '" << arg.Get() << "' could not be parsed." << std::endl;
  return false;
}

bool convert_crop (const Argument<std::string>& arg)
{
  int x,y, w, h, n;
  // parse
  
  // TODO: pretty C++ parser
  if ((n = sscanf(arg.Get().c_str(), "%d,%d,%d,%d", &x, &y, &w, &h)) == 4)
    {
      FOR_ALL_IMAGES(crop, x, y, w, h);
      return true;
    }
  std::cerr << "Crop '" << arg.Get() << "' could not be parsed." << std::endl;
  return false;
}

bool convert_fast_auto_crop (const Argument<bool>& arg)
{
  FOR_ALL_IMAGES(fastAutoCrop);
  return true;
}

bool convert_invert (const Argument<bool>& arg)
{
  FOR_ALL_IMAGES(invert);
  return true;
}

bool convert_deinterlace (const Argument<bool>& arg)
{
  // TODO: if this does what I think change to yield 2 images?
  FOR_ALL_IMAGES(deinterlace);
  return true;
}

/*
  #RGB                 (R,G,B are hex numbers, 4 bits each)
  #RRGGBB              (8 bits each)
  #RRRGGGBBB           (12 bits each)
  #RRRRGGGGBBBB        (16 bits each)
  
  TODO:

  #RGBA                (4 bits each)
  #RRGGBBOO            (8 bits each)
  #RRRGGGBBBOOO        (12 bits each)
  #RRRRGGGGBBBBOOOO    (16 bits each)
  
  X11, SVG standard colors
  name                 (identify -list color to see names)
  
  rgb(r,g,b)           0-255 for each of rgb
  rgba(r,g,b,a)        0-255 for each of rgb and 0-1 for alpha
  cmyk(c,m,y,k)        0-255 for each of cmyk
  cmyka(c,m,y,k,a)     0-255 for each of cmyk and 0-1 for alpha
  
  hsl(0, 100%, 50%) }

*/

static struct {
  const char* name;
  const char* color;
} named_colors[]  = {
  // CSS2
  { "white",   "#ffffff" },
  { "yellow",  "#ffff00" },
  { "orange",  "#ffA500" },
  { "red",     "#ff0000" },
  { "fuchsia", "#ff00ff" },
  { "silver",  "#c0c0c0" },
  { "gray",    "#808080" },
  { "olive",   "#808000" },
  { "purple",  "#800080" },
  { "maroon",  "#800000" },
  { "aqua",    "#00ffff" },
  { "lime",    "#00ff00" },
  { "teal",    "#008080" },
  { "green",   "#008000" },
  { "blue",    "#0000ff" },
  { "navy",    "#000080" },
  { "black",   "#000000" },
  
  // TODO: HTML, X11, SVG; ...
};

static bool parse_color (Image::iterator& it, const char* color)
{
  unsigned int r, g, b;
  int strl = strlen(color);
  
  if (strl == 4 && sscanf(color, "#%1x%1x%1x", &r, &g, &b) == 3)
    {
      double _r, _g, _b;
      _r = 1. / 0xf * r;
      _g = 1. / 0xf * g;
      _b = 1. / 0xf * b;
      it.setRGB(_r, _g, _b);
      return true;
    }
  else if (strl == 7 && sscanf(color, "#%2x%2x%2x", &r, &g, &b) == 3)
    {
      double _r, _g, _b;
      _r = 1. / 0xff * r;
      _g = 1. / 0xff * g;
      _b = 1. / 0xff * b;
      it.setRGB(_r, _g, _b);
      return true;
    }
  else if (strl == 10 && sscanf(color, "#%3x%3x%3x", &r, &g, &b) == 3)
    {
      double _r, _g, _b;
      _r = 1. / 0xfff * r;
      _g = 1. / 0xfff * g;
      _b = 1. / 0xfff * b;
      it.setRGB(_r, _g, _b);
      return true;
    }
  else if (strl == 13 && sscanf(color, "#%4x%4x%4x", &r, &g, &b) == 3)
    {
      double _r, _g, _b;
      _r = 1. / 0xffff * r;
      _g = 1. / 0xffff * g;
      _b = 1. / 0xffff * b;
      it.setRGB(_r, _g, _b);
      return true;
    }
  else for (unsigned i = 0; i < ARRAY_SIZE(named_colors); ++i)
    {
      if (strcmp(color, named_colors[i].name) == 0)
	return parse_color(it, named_colors[i].color);
    }
  return false;
}

bool convert_background (const Argument<std::string>& arg)
{
  if (!parse_color(background_color, arg.Get().c_str())) {
    std::cerr << "Error parsing color: '" << arg.Get() << "'" << std::endl;
    return false;
  }
  return true;
}

bool convert_foreground (const Argument<std::string>& arg)
{
  if (!parse_color(foreground_color, arg.Get().c_str())) {
    std::cerr << "Error parsing color: '" << arg.Get() << "'" << std::endl;
    return false;
  }
  return true;
}

bool convert_line (const Argument<std::string>& arg)
{
  unsigned int x1, y1, x2, y2;
  
  if (sscanf(arg.Get().c_str(), "%d,%d,%d,%d", &x1, &y1, &x2, &y2) == 4)
    {
      Path path;
      path.moveTo (x1, y1);
      path.addLineTo (x2, y2);
      
      double r = 0, g = 0, b = 0;
      foreground_color.getRGB (r, g, b);
      path.setFillColor (r, g, b);
      if (arg_stroke_width.Size())
	path.setLineWidth(arg_stroke_width.Get());
      FOR_ALL_IMAGES(path.draw);
      return true; 
    }
  
  std::cerr << "Error parsing line: '" << arg.Get() << "'" << std::endl;
  return false;
}

#if WITHFREETYPE == 1

bool convert_text (const Argument<std::string>& arg)
{
  double x = 0, y = 0, xoff = 0, yoff = 0, height = 0;
  char* text = 0; char* gravity = 0;
  if (sscanf(arg.Get().c_str(), "%lf,%lf,%lf,%a[^\r]",
	     &x, &y, &height, &text) != 4)
    if (sscanf(arg.Get().c_str(), "%a[^,],%lf,%a[^\r]",
	       &gravity, &height, &text) != 3) {
      std::cerr << "Error parsing text: '" << arg.Get() << "'" << std::endl;
      return false;
    }

  // parse gravity offset
  if (gravity)
  {
    char* gravity2 = 0;
    if (sscanf(gravity, "%a[^+-]%lf%lf",
	       &gravity2, &xoff, &yoff) == 3) {
      free(gravity);
      gravity = gravity2;
    }
  }
  
  for (images_iterator it = images.begin(); it != images.end(); ++it)
  {
    Path path;
    path.moveTo (0, 0);
    double r = 0, g = 0, b = 0;
    foreground_color.getRGB (r, g, b);
    path.setFillColor (r, g, b);

    const double strokeWidth = arg_stroke_width.Size() ? arg_stroke_width.Get() : 0;
    if (strokeWidth > 0)
      path.setLineWidth(strokeWidth);

    agg::trans_affine mtx;
    mtx *= agg::trans_affine_rotation(arg_text_rotation.Size() ?
                                      arg_text_rotation.Get() / 180 * M_PI : 0);

    if (gravity) {
      std::string c(gravity);
      std::transform (c.begin(), c.end(), c.begin(), tolower);
      
      enum align {A, B, C};
      align x_align = A, y_align = A;
      
      if (c == "northwest") {
	x_align = A; y_align = A;
      }
      else if (c == "north" || c == "top") {
	x_align = B; y_align = A;
      }
      else if (c == "northeast") {
	x_align = C; y_align = A;
      }
      else if (c == "west" || c == "left") {
	x_align = A; y_align = B;
      }
      else if (c == "center") {
	x_align = B; y_align = B;
      }
      else if (c == "east" || c == "right") {
	x_align = C; y_align = B;
      }
      else if (c == "southwest") {
	x_align = A; y_align = C;
      }
      else if (c == "south" || c == "bottom") {
	x_align = B; y_align = C;
      }
      else if (c == "southeast") {
	x_align = C; y_align = C;
      }
      else {
	std::cerr << "Unknown gravity: " << gravity << std::endl;
	return false;
      }
      
      double w = 0, h = 0, dx = 0, dy = 0;
      path.drawText(**it, text, height,
		    arg_font.Size() ? arg_font.Get().c_str() : NULL, mtx,
		    strokeWidth > 0 ? Path::fill_none : Path::fill_non_zero,
		    &w, &h, &dx, &dy);
      
      switch (x_align) {
      case A: x = 0;
	break;
      case B: x = ((*it)->w - w) / 2;
	break;
      case C: x = (*it)->w - w;
	break;
      }
      
      switch (y_align) {
      case A: y = 0;
	break;
      case B: y = ((*it)->h - h) / 2;
	break;
      case C: y = (*it)->h - h;
	break;
      }

      mtx *= agg::trans_affine_translation(dx + x + xoff, dy + y + yoff);
      path.drawText(**it, text, height,
		    arg_font.Size() ? arg_font.Get().c_str() : NULL, mtx,
		    strokeWidth > 0 ? Path::fill_none : Path::fill_non_zero);
    }
    else {
      mtx *= agg::trans_affine_translation(x + xoff, y + yoff);
      path.drawText(**it, text, height,
		    arg_font.Size() ? arg_font.Get().c_str() : NULL, mtx,
		    strokeWidth > 0 ? Path::fill_none : Path::fill_non_zero);
    }
  }
  
  free(text); free(gravity);
  return true; 
}

#endif

int main (int argc, char* argv[])
{
  ArgumentList arglist;
  background_color.type = Image::RGB8;
  background_color.setL (255);
  foreground_color.type = Image::RGB8;
  foreground_color.setL (0);
  
  // setup the argument list
  Argument<bool> arg_help ("h", "help",
			   "display this help text and exit");
  arglist.Add (&arg_help);
  
  Argument<std::string> arg_input ("i", "input",
				   "input file or '-' for stdin, optionally prefixed with format:"
				   "\n\t\te.g: jpg:- or raw:rgb8-dump",
                                   0, std::numeric_limits<int>::max(), true, true);
  arg_input.Bind (convert_input);
  arglist.Add (&arg_input);
  
  Argument<std::string> arg_output ("o", "output",
				    "output file or '-' for stdout, optinally prefix with format:"
				    "\n\t\te.g. jpg:- or raw:rgb8-dump",
				    0, std::numeric_limits<int>::max(), true, true);
  arg_output.Bind (convert_output);
  arglist.Add (&arg_output);

  // TODO: more args for the stack?
  Argument<std::string> arg_append ("a", "append",
				   "append file or '-' for stdin, optionally prefixed with format:"
				   "\n\t\te.g: jpg:- or raw:rgb8-dump",
                                   0, 1, true, true);
  arg_append.Bind (convert_append);
  arglist.Add (&arg_append);

  
  // global
  arglist.Add (&arg_quality);
  arglist.Add (&arg_compression);
  arglist.Add (&arg_decompression);
  
  Argument<std::string> arg_split ("", "split",
			   "filenames to save the images split in Y-direction into n parts",
			   0, 1, true, true);
  arg_split.Bind (convert_split);
  arglist.Add (&arg_split);
  
  Argument<std::string> arg_colorspace ("", "colorspace",
					"convert image colorspace (BW, BILEVEL, GRAY, GRAY1, GRAY2, GRAY4,\n\t\tRGB, YUV, CYMK)",
					0, 1, true, true);
  arg_colorspace.Bind (convert_colorspace);
  arglist.Add (&arg_colorspace);

  Argument<bool> arg_normalize ("", "normalize",
				"transform the image to span the full color range",
				0, 0, true, true);
  arg_normalize.Bind (convert_normalize);
  arglist.Add (&arg_normalize);

  Argument<double> arg_brightness ("", "brightness",
				   "change image brightness",
				   0.0, 0, 1, true, true);
  arg_brightness.Bind (convert_brightness);
  arglist.Add (&arg_brightness);

  Argument<double> arg_contrast ("", "contrast",
				 "change image contrast",
				 0.0, 0, 1, true, true);
  arg_contrast.Bind (convert_contrast);
  arglist.Add (&arg_contrast);

  Argument<double> arg_gamma ("", "gamma",
				 "change image gamma",
				 0.0, 0, 1, true, true);
  arg_gamma.Bind (convert_gamma);
  arglist.Add (&arg_gamma);

  Argument<double> arg_hue ("", "hue",
				 "change image hue",
				 0.0, 0, 1, true, true);
  arg_hue.Bind (convert_hue);
  arglist.Add (&arg_hue);

  Argument<double> arg_saturation ("", "saturation",
				   "change image saturation",
				   0.0, 0, 1, true, true);
  arg_saturation.Bind (convert_saturation);
  arglist.Add (&arg_saturation);

  Argument<double> arg_lightness ("", "lightness",
				  "change image lightness",
				  0.0, 0, 1, true, true);
  arg_lightness.Bind (convert_lightness);
  arglist.Add (&arg_lightness);

  Argument<double> arg_blur ("", "blur",
				 "gaussian blur",
				 0.0, 0, 1, true, true);
  arg_blur.Bind (convert_blur);
  arglist.Add (&arg_blur);

  
  Argument<double> arg_scale ("", "scale",
			      "scale image data using a method suitable for specified factor",
			      0.0, 0, 1, true, true);
  arg_scale.Bind (convert_scale);
  arglist.Add (&arg_scale);
  
  Argument<double> arg_nearest_scale ("", "nearest-scale",
				   "scale image data to nearest neighbour",
				   0.0, 0, 1, true, true);
  arg_nearest_scale.Bind (convert_nearest_scale);
  arglist.Add (&arg_nearest_scale);

  Argument<double> arg_bilinear_scale ("", "bilinear-scale",
				       "scale image data with bi-linear filter",
				       0.0, 0, 1, true, true);
  arg_bilinear_scale.Bind (convert_bilinear_scale);
  arglist.Add (&arg_bilinear_scale);

  Argument<double> arg_bicubic_scale ("", "bicubic-scale",
				      "scale image data with bi-cubic filter",
				      0.0, 0, 1, true, true);
  arg_bicubic_scale.Bind (convert_bicubic_scale);
  arglist.Add (&arg_bicubic_scale);

  Argument<double> arg_ddt_scale ("", "ddt-scale",
				      "scale image data with data dependent triangulation",
				      0.0, 0, 1, true, true);
  arg_ddt_scale.Bind (convert_ddt_scale);
  arglist.Add (&arg_ddt_scale);
  
  Argument<double> arg_box_scale ("", "box-scale",
				   "(down)scale image data with box filter",
				  0.0, 0, 1, true, true);
  arg_box_scale.Bind (convert_box_scale);
  arglist.Add (&arg_box_scale);

  Argument<double> arg_thumbnail_scale ("", "thumbnail",
					"quick and dirty down-scale for a thumbnail",
					0.0, 0, 1, true, true);
  arg_thumbnail_scale.Bind (convert_thumbnail_scale);
  arglist.Add (&arg_thumbnail_scale);

   Argument<double> arg_rotate ("", "rotate",
			       "rotation angle",
			       0, 1, true, true);
  arg_rotate.Bind (convert_rotate);
  arglist.Add (&arg_rotate);

  Argument<double> arg_convolve ("", "convolve",
			       "convolution matrix",
			       0, 9999, true, true);
  arg_convolve.Bind (convert_convolve);
  arglist.Add (&arg_convolve);

  Argument<bool> arg_flip ("", "flip",
			   "flip the image vertically",
			   0, 0, true, true);
  arg_flip.Bind (convert_flip);
  arglist.Add (&arg_flip);

  Argument<bool> arg_flop ("", "flop",
			   "flip the image horizontally",
			   0, 0, true, true);
  arg_flop.Bind (convert_flop);
  arglist.Add (&arg_flop);

  Argument<int> arg_floyd ("", "floyd-steinberg",
			   "Floyd Steinberg dithering using n shades",
			   0, 1, true, true);
  arg_floyd.Bind (convert_dither_floyd_steinberg);
  arglist.Add (&arg_floyd);
  
  Argument<int> arg_riemersma ("", "riemersma",
			       "Riemersma dithering using n shades",
			       0, 1, true, true);
  arg_riemersma.Bind (convert_dither_riemersma);
  arglist.Add (&arg_riemersma);


  Argument<bool> arg_edge ("", "edge",
                           "edge detect filter",
			   0, 0, true, true);
  arg_edge.Bind (convert_edge);
  arglist.Add (&arg_edge);
  
  Argument<std::string> arg_resolution ("", "resolution",
					"set meta data resolution in dpi to x[xy] e.g. 200 or 200x400",
					0, 1, true, true);
  arg_resolution.Bind (convert_resolution);
  arglist.Add (&arg_resolution);

  Argument<std::string> arg_size ("", "size",
			      "width and height of raw images whose dimensions are unknown",
			      0, 1, true, true);
  arg_size.Bind (convert_size);
  arglist.Add (&arg_size);

  Argument<std::string> arg_crop ("", "crop",
			      "crop an area out of an image: x,y,w,h",
			      0, 1, true, true);
  arg_crop.Bind (convert_crop);
  arglist.Add (&arg_crop);

  Argument<bool> arg_fast_auto_crop ("", "fast-auto-crop",
				     "fast auto crop",
				     0, 0, true, true);
  arg_fast_auto_crop.Bind (convert_fast_auto_crop);
  arglist.Add (&arg_fast_auto_crop);

  Argument<bool> arg_invert ("", "negate",
                             "negates the image",
                               0, 0, true, true);
  arg_invert.Bind (convert_invert);
  arglist.Add (&arg_invert);

  Argument<bool> arg_deinterlace ("", "deinterlace",
				  "shuffle every 2nd line",
				  0, 0, true, true);
  arg_deinterlace.Bind (convert_deinterlace);
  arglist.Add (&arg_deinterlace);

  Argument<std::string> arg_background ("", "background",
					"background color used for operations",
					0, 1, true, true);
  arg_background.Bind (convert_background);
  arglist.Add (&arg_background);
  
  Argument<std::string> arg_foreground ("", "foreground",
					"foreground color used for operations",
					0, 1, true, true);
  arg_foreground.Bind (convert_foreground);
  arglist.Add (&arg_foreground);

  arglist.Add (&arg_stroke_width);
 
  Argument<std::string> arg_line ("", "line",
                                  "draw a line: x1, y1, x2, y2",
                                   0, 1, true, true);
  arg_line.Bind (convert_line);
  arglist.Add (&arg_line);

#if WITHFREETYPE == 1
  Argument<std::string> arg_text ("", "text",
                                  "draw text: x1, y1, height, text",
				  0, 1, true, true);
  arg_text.Bind (convert_text);
  arglist.Add (&arg_text);
  arglist.Add (&arg_font);
  arglist.Add (&arg_text_rotation);
#endif
  
  // parse the specified argument list - and maybe output the Usage
  if (!arglist.Read (argc, argv)) {
    freeImages();
    return 1;
  }
  
  // print the usage if no argument was given or help requested
  if (argc == 1 || arg_help.Get() == true)
    {
      std::cerr << "ExactImage converter, version " VERSION << std::endl
		<< "Copyright (C) 2005 - 2011 René Rebe, ExactCODE" << std::endl
		<< "Copyright (C) 2005, 2008 Archivista" << std::endl
		<< "Usage:" << std::endl;
      
      arglist.Usage (std::cerr);
      freeImages();
      return 1;
    }
  
  // stack: insert, append, delete, swap, clone
  
  // all is done inside the argument callback functions
  freeImages();
  return 0;
}
