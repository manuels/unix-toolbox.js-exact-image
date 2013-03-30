/*
 * Copyright (C) 2007 - 2009 René Rebe
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

#include <ctype.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "xpm.hh"

#include "Colorspace.hh"

// Format:
//
// /* XPM */"
// static char * XFACE[] = {
// "48 48 2 1",
// "a c #ffffff",
// "b c #000000",
// "abaabaababaaabaabababaabaabaababaabaaababaabaaab"
// ...

uint8_t parse_hex (std::istream* stream)
{
  uint8_t x = 0;

  char c = tolower (stream->get());
  if (c >= '0' && c <= '9')
    x = (x << 4) | (c - '0');
  else
    x = (x << 4) | (c - 'a' + 10);
  
  c = tolower (stream->get());
  if (c >= '0' && c <= '9')
    x = (x << 4) | (c - '0');
  else
    x = (x << 4) | (c - 'a' + 10);

  return x;
}

void skip_comments (std::istream* stream)
{
  if (stream->peek() == '/')
    {
      stream->get();
      if (stream->peek() == '*')
	{
	  // std::cerr << "comment" << std::endl;
	  do {
	    char c = stream->get();
	    // std::cerr << c;
	    if (c == '*' && stream->peek() == '/') {
	      stream->get();
	      break;
	    }
	  } while (*stream);
	  while (*stream && stream->peek() == '\n')
	    stream->get();
	  // std::cerr << std::endl;
	}
      else
	stream->putback('/');
    }
}

int XPMCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
  // check signature
  std::string line;
  std::getline (*stream, line);
  if (line != "/* XPM */") {
    stream->seekg (0);
    return false;
  }
  
  // TODO: wirte a more sophisticated parser here
  
  // drop the C declaration
  std::getline (*stream, line);
  
  skip_comments (stream);
  // dimensions and #colors
  if (stream->peek() == '"')
    stream->get();
  
  int colors, cpp; // chars per pixel
  *stream >> image.w >> image.h >> colors >> cpp;
  std::getline (*stream, line);

  if (false)
  std::cerr << "XPM: " << image.w << "x" << image.h
	    << ", colors: " << colors << ", chars per pix: "
	    << cpp << std::endl;
  
  uint16_t* rmap = new uint16_t [colors];
  uint16_t* gmap = new uint16_t [colors];
  uint16_t* bmap = new uint16_t [colors];
  std::vector<std::string> cmap;
  
  skip_comments (stream);
  
  // read color definitions
  for (int c = 0; c < colors; ++c)
    {
      std::string name, type;
  
      // std::cerr << "Reading color: " << c << std::endl;
      
      if (stream->peek() == '"')
	stream->get();
      
      for (int i = 0; i < cpp; ++i)
	name.push_back (stream->get());
      
      *stream >> type;
      // Type: c -> colour, m -> monochrome, g -> grayscale, and s -> symbolic
      if (type != "c") {
	std::cerr << "XPM color type: " << type << " not yet implemented." << std::endl;
	return false;
      }
      
      while (stream->peek() == ' ')
	stream->get();
      
      if (stream->peek() == '#') {
	stream->get();
	rmap[c] = parse_hex (stream) << 8;
	gmap[c] = parse_hex (stream) << 8;
	bmap[c] = parse_hex (stream) << 8;
	std::getline (*stream, line);
      }
      else {
	rmap[c] = gmap[c] = bmap[c] = 0;
	std::getline (*stream, line);
	if (line != "None\",")
	  std::cerr << "Unrecognized color: " << line << std::endl;
      }
      
      // symbol -> index map
      cmap.push_back (name);
      //std::cerr << cmap[c] << ": " << rmap[c] << " " << gmap[c] << " " << bmap[c] << std::endl;
    }
  
  image.bps = 8; // for now, later this could be optimized
  image.spp = 3; // for now, later this could be optimized
  image.resize (image.w, image.h);
  image.setResolution(0, 0);
  
  skip_comments (stream);

  // read in the pixel data
  uint8_t* dst = image.getRawData();
  
  int c = 0;
  std::string last = "";
  for (int y = 0; y < image.h; ++y)
    {
      if (stream->peek() == '"')
	stream->get();
      for (int x = 0; x < image.w; ++x)
	{
	  std::string str;
	  for (int i = 0; i < cpp; ++i)
	    str.append (1, (char)stream->get());
	  
	  if (str != last) {
	    // TODO: make this a hash lookup ...
	    std::vector<std::string>::iterator it =
	      find (cmap.begin(), cmap.end(), str);
	    
	    if (it == cmap.end())
	      std::cerr << "Not in color map: '" << str << "'!" << std::endl;
	    else {
	      c = (it - cmap.begin());
	      last = str;
	    }
	  }
	  
	  *++dst = c;
	}
      std::getline (*stream, line);
    }
  
  colorspace_de_palette (image, colors, rmap, gmap, bmap);
  delete (rmap); delete (gmap); delete (bmap);
  rmap = gmap = bmap = 0;

  return true;
}

std::string symbol (int i)
{
  std::string s;
  s.append (1, (char) ('a' + i));
  return s;
}

std::string put_hex (unsigned char hi)
{
  std::string s;
  
  unsigned char lo = hi & 0xF;
  hi >>= 4;
  
  if (hi < 10)
    s.append (1, (char)'0' + hi);
  else
    s.append (1, (char)'A' + hi - 10);

  if (lo < 10)
    s.append (1, (char)'0' + lo);
  else
    s.append (1, (char)'A' + lo - 10);
  
  return s;
}

bool XPMCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
  // TODO: count colors later
  if (image.spp > 1) {
    std::cerr << "Too many colors for XPM." << std::endl;
    return false;
  }
  
  int colors = (1 << image.bps);
      
  *stream << "/* XPM */\n"
	  << "static char * ExactImage[] = {\n"
	  << "\"" << image.w << " " << image.h << " "
	  << colors  << " " << 1 /* cpp */ << "\",\n";
  
  // write colors
  for (int c = 0; c < colors; ++c)
    {
      int l = c * 255 / (colors-1);
      *stream << "\"" << symbol (c) << "\t" << "c #"
	      << put_hex(l)
	      << put_hex(l)
	      << put_hex(l)
	      << "\",\n";
    }
  
  // write pixels
  Image::iterator it = image.begin ();
  for (int y = 0; y < image.h; ++y)
    {
      *stream << "\"";
      for (int x = 0; x < image.w; ++x)
	{
	  *it;
	  *stream << symbol (it.getL() >> (8 - image.bps) );
	  ++it;
	}
      
      *stream << "\"" << (y < image.h-1 ? ",\n" : "};\n");
    }

  return true;
}

XPMCodec xpm_loader;
