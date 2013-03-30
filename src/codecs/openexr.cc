/*
 * Copyright (C) 2006 - 2008 René Rebe
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ImfIO.h>

#include <ImfInputFile.h>
#include <ImfOutputFile.h>

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <IexThrowErrnoExc.h>

#include <algorithm>

#include "openexr.hh"

#include "Colorspace.hh"

using namespace Imf;
using namespace Imath;

using std::cout;
using std::endl;

class STDIStream : public IStream
{
public:
  STDIStream (std::istream* stream, const char fileName[])
    : IStream (fileName), _stream (stream) {
  }
  
  virtual bool read (char c[], int n)
  {
    try {
    _stream->read (c, n);
    }
    catch (...)
      {
	if (!*_stream)
	  Iex::throwErrnoExc();
	else
	  throw Iex::InputExc ("Unexpected end of file.");
      }
    return _stream->eof();
  }
    
  virtual Int64 tellg ()
  {
    return _stream->tellg ();
  }
  
  virtual void seekg (Int64 pos)
  {
    _stream->clear ();
    _stream->seekg (pos);
  }
  
  virtual void clear ()
  {
    _stream->clear ();
  }
  
private:
  std::istream* _stream;
};

class STDOStream : public OStream
{
public:
  STDOStream (std::ostream* stream, const char fileName[])
    : OStream (fileName),  _stream (stream) {
  }
  
  virtual void write (const char c[], int n)
  {
    try {
      _stream->write (c, n);
    }
    catch (...)
      {
	if (!*_stream)
	  Iex::throwErrnoExc();
	else
	  throw Iex::InputExc ("Unexpected end of file.");
      }
  }
    
  virtual Int64 tellp ()
  {
    return _stream->tellp ();
  }
  
  virtual void seekp (Int64 pos)
  {
    _stream->clear ();
    _stream->seekp (pos);
  }
  
  virtual void clear ()
  {
    _stream->clear ();
  }
  
private:
  std::ostream* _stream;
};

int OpenEXRCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
  STDIStream istream (stream, "");
  
  {
    // quick magic check
    char buf [3];
    stream->read (buf, sizeof (buf));
    stream->seekg (0);
    
    // inaccurate, but enough for now
    if (buf[0] != 'v' || buf[1] != '/' || buf[2] != '1')
      return false;
  }
  
try {
  RgbaInputFile exrfile (istream);
  Box2i dw = exrfile.dataWindow ();
  
  image.spp = 4;
  image.bps = 16;
  
  image.resize (dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1);
  
  Array2D<Rgba> pixels (1, image.w); // working data
  
  uint16_t* it = (uint16_t*) image.getRawData();
  for (int y = 0; y < image.h; ++y)
    {
      exrfile.setFrameBuffer (&pixels[0][0] - y * image.w, 1, image.w);
      exrfile.readPixels (y, y);
      
      for (int x = 0; x < image.w; ++x) {
	double r = pixels[0][x].r;
	double g = pixels[0][x].g;
	double b = pixels[0][x].b;
	double a = pixels[0][x].a;
	
	r = std::min (std::max (r,0.0),1.0) * 0xFFFF;
	g = std::min (std::max (g,0.0),1.0) * 0xFFFF;
	b = std::min (std::max (b,0.0),1.0) * 0xFFFF;
	a = std::min (std::max (a,0.0),1.0) * 0xFFFF;
	
	*it++ = (int) r; *it++ = (int)g; *it++ = (int)b; *it++ = (int)a;
      }
    }
  
  return true;
 } catch (...) {
  return false;
 }
}

bool OpenEXRCodec::writeImage (std::ostream* stream, Image& image,
			       int quality, const std::string& compress)
{
  RgbaChannels type;
  switch (image.spp) {
  case 1:
    type = WRITE_Y; break;
  case 2:
    type = WRITE_YA; break;
  case 3:
    type = WRITE_RGB; break;
  case 4:
    type = WRITE_RGBA; break;
    break;
  default:
    std::cerr << "Unsupported image format." << std::endl;
    return false;
  }
      
  
  Box2i displayWindow (V2i (0, 0), V2i (image.w - 1, image.h - 1));
  
  STDOStream ostream (stream, "");
    
  Header header (image.w, image.h);
  RgbaOutputFile exrfile (ostream, header, type);
  
  Array2D<Rgba> pixels (1, image.w); // working data
  
  uint16_t* it = (uint16_t*) image.getRawData();
  for (int y = 0; y < image.h; ++y)
    {
      exrfile.setFrameBuffer (&pixels[0][0] - y * image.w, 1, image.w);
      
      for (int x = 0; x < image.w; ++x) {
	pixels[0][x].r = (double)*it++ / 0xFFFF;
	pixels[0][x].g = (double)*it++ / 0xFFFF;
	pixels[0][x].b = (double)*it++ / 0xFFFF;
	pixels[0][x].a = (double)*it++ / 0xFFFF;
      }
      
      exrfile.writePixels (1);
    }
  
  return true;
}

OpenEXRCodec openexr_loader;
