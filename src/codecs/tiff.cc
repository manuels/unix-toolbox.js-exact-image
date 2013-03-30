/*
 * Copyright (C) 2005 - 2011 René Rebe, ExactCODE GmbH, Berlin
 *           (C) 2005 Archivista GmbH, CH-8042 Zuerich
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

#include "tiff.hh"

#include "Colorspace.hh"

#include <algorithm>
#include <iostream>

/* Well, sadly our own c++ glue, the libtiff native one does not provide
   readble out streams it requires for multi-page files, ... */

static const bool trace = false;

extern "C" {
  
  struct tiffis_data
  {
    std::istream* IS;
    long streamStartPos;
  };

  struct tiffos_data
  {
    std::ostream* OS;
    long streamStartPos;
  };

  static tsize_t _tiffisReadProc(thandle_t fd, tdata_t buf, tsize_t size)
  {
    tiffis_data* data = (tiffis_data*)fd;

    data->IS->read((char*)buf, (int)size);

    return data->IS->gcount();
  }

  static tsize_t _tiffosReadProc(thandle_t fd, tdata_t buf, tsize_t size)
  {
    if (trace)
      std::cerr << __FUNCTION__ << " " << size << std::endl;

    tiffos_data* data = (tiffos_data*)fd;
    std::istream* is = dynamic_cast<std::istream*>(data->OS);
    if (is)
      {
	is->sync();
	if (trace)
	  std::cerr << is->tellg() << std::endl;
	is->read((char*)buf, (int)size);
	long c = is->gcount();
	if (trace)
	  std::cerr << is->tellg() << std::endl;;
	if (trace)
	  std::cerr << c << std::endl;
	return c;
      }
    else
      return 0;
  }

  static tsize_t _tiffosWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
  {
    tiffos_data* data = (tiffos_data*)fd;
    std::ostream* os = data->OS;
    int pos = os->tellp();

    if (trace)
      std::cerr << __FUNCTION__ << " " << size << " " << os->fail() << std::endl;

    os->write((const char*)buf, size);

    int c = ((int)os->tellp()) - pos;
    if (trace)
      std::cerr << c << std::endl;
    return c;
  }

  static tsize_t _tiffisWriteProc(thandle_t, tdata_t, tsize_t)
  {
    return 0;
  }

  static toff_t _tiffosSeekProc(thandle_t fd, toff_t off, int whence)
  {
    if (trace)
      std::cerr << __FUNCTION__ << " " << off << std::endl;

    tiffos_data* data = (tiffos_data*)fd;
    std::ostream* os = data->OS;
  
    // according to my tests g and p pointer are changed in-sync
  
    // if the stream has already failed, don't do anything
    if (os->fail())
      os->clear();
  
    if (trace)
      std::cerr << "failed: " << os->fail() << std::endl;
  
    switch (whence) {
    case SEEK_SET:
      if (trace)
	std::cerr << "SET " << data->streamStartPos << " " <<  off << std::endl;
      os->seekp(data->streamStartPos + off, std::ios::beg);
      break;
    case SEEK_CUR:
      os->seekp(off, std::ios::cur);
      break;
    case SEEK_END:
      os->seekp(off, std::ios::end);
      break;
    }

    if (trace)
      std::cerr << "failed: " << os->fail() << std::endl;
  
    // Attempt to workaround problems with seeking past the end of the
    // stream.  ofstream doesn't have a problem with this but
    // ostrstream/ostringstream does. In that situation, add intermediate
    // '\0' characters.
    if (os->fail()) {
      std::ios::iostate old_state;
      toff_t origin = 0;

      old_state = os->rdstate();
      // reset the fail bit or else tellp() won't work below
      os->clear(os->rdstate() & ~std::ios::failbit);
      switch (whence) {
      case SEEK_SET:
	origin = data->streamStartPos;
	break;
      case SEEK_CUR:
	origin = os->tellp();
	break;
      case SEEK_END:
	os->seekp(0, std::ios::end);
	origin = os->tellp();
	break;
      }
      // restore original stream state
      os->clear(old_state);	

      // only do something if desired seek position is valid
      if (origin + off > data->streamStartPos) {
	toff_t num_fill;
      
	// clear the fail bit 
	os->clear(os->rdstate() & ~std::ios::failbit);

	// extend the stream to the expected size
	os->seekp(0, std::ios::end);
	num_fill = origin + off - (toff_t)os->tellp();
	for (toff_t i = 0; i < num_fill; i++)
	  os->put('\0');

	// retry the seek
	os->seekp(origin + off, std::ios::beg);
      }
    }

    int c = os->tellp();
    if (trace)
      std::cerr << c << " failed: " << os->fail() << std::endl;
    return c;
  }

  static toff_t _tiffisSeekProc(thandle_t fd, toff_t off, int whence)
  {
    tiffis_data* data = (tiffis_data*)fd;

    switch (whence) {
    case SEEK_SET:
      data->IS->seekg(data->streamStartPos + off, std::ios::beg);
      break;
    case SEEK_CUR:
      data->IS->seekg(off, std::ios::cur);
      break;
    case SEEK_END:
      data->IS->seekg(off, std::ios::end);
      break;
    }

    return ((long)data->IS->tellg()) - data->streamStartPos;
  }

  static toff_t _tiffosSizeProc(thandle_t fd)
  {
    if (trace)
      std::cerr << __FUNCTION__ << std::endl;

    tiffos_data	*data = (tiffos_data*)fd;
    std::ostream		*os = data->OS;
    toff_t		pos = os->tellp();
    toff_t		len;

    os->seekp(0, std::ios::end);
    len = os->tellp();
    os->seekp(pos);

    return len;
  }

  static toff_t _tiffisSizeProc(thandle_t fd)
  {
    tiffis_data	*data = (tiffis_data*)fd;
    int		pos = data->IS->tellg();
    int		len;

    data->IS->seekg(0, std::ios::end);
    len = data->IS->tellg();
    data->IS->seekg(pos);

    return len;
  }

  static int _tiffosCloseProc(thandle_t fd)
  {
    if (trace)
      std::cerr << __FUNCTION__ << std::endl;

    delete (tiffos_data*)fd;
    return 0;
  }

  static int _tiffisCloseProc(thandle_t fd)
  {
    delete (tiffis_data*)fd;
    return 0;
  }

  static int _tiffDummyMapProc(thandle_t , tdata_t* , toff_t*)
  {
    if (trace)
      std::cerr << __FUNCTION__ << std::endl;

    return (0);
  }

  static void _tiffDummyUnmapProc(thandle_t , tdata_t , toff_t)
  {
  }

} // end. extern "C"

static TIFF* _tiffStreamOpen(const char* name, const char* mode, void* fd)
{
  TIFF*	tif;

  if (strchr(mode, 'w')) {
    tiffos_data* data = new tiffos_data;
    data->OS = (std::ostream*)fd;
    data->streamStartPos = data->OS->tellp();
    if (data->streamStartPos < 0)
      data->streamStartPos = 0;
    tif = TIFFClientOpen(name, mode,
			 (thandle_t) data,
			 _tiffosReadProc, _tiffosWriteProc,
			 _tiffosSeekProc, _tiffosCloseProc,
			 _tiffosSizeProc,
			 _tiffDummyMapProc, _tiffDummyUnmapProc);
  } else {
    tiffis_data* data = new tiffis_data;
    data->IS = (std::istream*)fd;
    data->streamStartPos = data->IS->tellg();
    if (data->streamStartPos < 0)
      data->streamStartPos = 0;
    // Open for reading.
    tif = TIFFClientOpen(name, mode,
			 (thandle_t) data,
			 _tiffisReadProc, _tiffisWriteProc,
			 _tiffisSeekProc, _tiffisCloseProc,
			 _tiffisSizeProc,
			 _tiffDummyMapProc, _tiffDummyUnmapProc);
  }
  
  if (trace)
    std::cerr << tif << std::endl;
  return tif;
}

static TIFF* TIFFStreamOpen(const char* name, std::ostream* os)
{
  // If os is either a ostrstream or ostringstream, and has no data
  // written to it yet, then tellp() will return -1 which will break us.
  // We workaround this by writing out a dummy character and
  // then seek back to the beginning.
  if (trace)
    std::cerr << "test: " << os->tellp() << std::endl;
  if (!os->fail() && (int)os->tellp() < 0) {
    *os << '\0';
    os->seekp(1);
    
    if (trace)
      std::cerr << "test: " << os->tellp() << std::endl;
    if (trace)
      std::cerr << "wrote a dummy" << std::endl;
  }
  
  return _tiffStreamOpen(name, "wm", os); // m for no mmap
}

static TIFF* TIFFStreamOpen(const char* name, std::istream* is)
{
  // NB: We don't support mapped files with streams so add 'm'
  return _tiffStreamOpen(name, "rm", is); // m for no mmap
}

/* back to our codec */

TIFCodec::TIFCodec () : tiffCtx(0) {
  registerCodec ("tiff", this);
  registerCodec ("tif", this);
}

TIFCodec::TIFCodec (TIFF* ctx) : tiffCtx(ctx) {
}

TIFCodec::~TIFCodec()
{
  if (tiffCtx)
    TIFFClose(tiffCtx);
}

int TIFCodec::readImage (std::istream* stream, Image& image, const std::string& decompres, int index)
{
  TIFF* in;

  // quick magic check
  {
    char a, b;
    a = stream->get ();
    b = stream->peek ();
    stream->putback (a);
    
    int magic = (a << 8) | b;
    
    if (magic != TIFF_BIGENDIAN && magic != TIFF_LITTLEENDIAN)
      return false;
  }
  
  in = TIFFStreamOpen ("", stream);
  if (!in)
    return false;

  int n_images = TIFFNumberOfDirectories(in);
  if (index > 0 || index != TIFFCurrentDirectory(in))
    if (!TIFFSetDirectory(in, index)) {
      TIFFClose(in);
      return false;
    }
  
  uint16 photometric = 0;
  TIFFGetField(in, TIFFTAG_PHOTOMETRIC, &photometric);
  // std::cerr << "photometric: " << (int)photometric << std::endl;
  switch (photometric)
    {
    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_RGB:
    case PHOTOMETRIC_PALETTE:
      break;
    default:
      std::cerr << "TIFCodec: Unrecognized photometric: " << (int)photometric << std::endl;
      TIFFClose(in);
      return false;
    }
  
  uint32 _w = 0;
  TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &_w);
  
  uint32 _h = 0;
  TIFFGetField(in, TIFFTAG_IMAGELENGTH, &_h);
  
  uint16 _spp = 0;
  TIFFGetField(in, TIFFTAG_SAMPLESPERPIXEL, &_spp);
  
  uint16 _bps = 0;
  TIFFGetField(in, TIFFTAG_BITSPERSAMPLE, &_bps);
  
  if (!_w || !_h || !_spp || !_bps) {
    TIFFClose(in);
    stream->seekg(0);
    return false;
  }
  
  //uint16 config;
  //TIFFGetField(in, TIFFTAG_PLANARCONFIG, &config);
  
  image.w = _w;
  image.h = _h;
  image.spp = _spp;
  image.bps = _bps;
  
  float _xres, _yres;
  if (!TIFFGetField(in, TIFFTAG_XRESOLUTION, &_xres))
    _xres = 0;
  if (!TIFFGetField(in, TIFFTAG_YRESOLUTION, &_yres))
    _yres = 0;
  image.setResolution(_xres, _yres);
  
  int stride = image.stride();
  image.resize (image.w, image.h);
  
  uint16 *rmap = 0, *gmap = 0, *bmap = 0;
  if (photometric == PHOTOMETRIC_PALETTE)
    {
      if (!TIFFGetField (in, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap))
	std::cerr << "TIFCodec: Error reading colormap." << std::endl;
    }

  uint8_t* data2 = image.getRawData();
  for (int row = 0; row < image.h; row++)
    {
      if (TIFFReadScanline(in, data2, row, 0) < 0)
	break;
      
      if (photometric == PHOTOMETRIC_MINISWHITE && image.bps == 1)
	for (int i = 0; i < stride; ++i)
	  data2[i] = data2[i] ^ 0xFF;
      
      data2 += stride;
    }
  
  /* some post load fixup */
  
  /* invert if saved "inverted", we already invert 1bps on-the-fly */
  if (photometric == PHOTOMETRIC_MINISWHITE && image.bps != 1)
    invert (image);
  
  /* strange 16bit gray images ??? or GRAYA? */
  if (image.spp == 2)
    {
      for (uint8_t* it = image.getRawData();
	   it < image.getRawDataEnd(); it += 2) {
	char x = it[0];
	it[0] = it[1];
	it[1] = x;
      }
      
      image.spp = 1;
      image.bps *= 2;
    }
  
  if (photometric == PHOTOMETRIC_PALETTE) {
    colorspace_de_palette (image, 1 << image.bps, rmap, gmap, bmap);
    /* free'd by TIFFClose; free(rmap); free(gmap); free(bmap); */
  }
  
  TIFFClose (in);
  return n_images;
}

// for multi-page writing
ImageCodec* TIFCodec::instanciateForWrite (std::ostream* stream)
{
  TIFF* out = TIFFStreamOpen ("", stream);
  if (out == NULL)
    return 0;

  return new TIFCodec(out);
}

bool TIFCodec::Write (Image& image,
		      int quality, const std::string& compress, int index)
{
  return writeImageImpl (tiffCtx, image, compress, index);
}

bool TIFCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
  TIFF* out = TIFFStreamOpen ("", stream);
  if (out == NULL)
    return false;
  
  bool ret = writeImageImpl (out, image, compress, 0);
  TIFFClose (out);
  
  return ret;
}

bool TIFCodec::writeImageImpl (TIFF* out, const Image& image, const std::string& compress,
			       int page)
{
  uint32 rowsperstrip = (uint32)-1;
  
  uint16 compression = image.bps == 1 ? COMPRESSION_CCITTFAX4 :
                                        COMPRESSION_DEFLATE;

  if (!compress.empty())
  {
    std::string c (compress);
    std::transform (c.begin(), c.end(), c.begin(), tolower);
    
    if (c == "g3" || c == "fax" || c == "group3")
      compression = COMPRESSION_CCITTFAX3;
    else if (c == "g4" || c == "group4")
      compression = COMPRESSION_CCITTFAX4;
    else if (c == "lzw")
      compression = COMPRESSION_LZW;
    else if (c == "deflate" || c == "zip")
      compression = COMPRESSION_DEFLATE;
    else if (c == "packbits")
      compression = COMPRESSION_PACKBITS;
    else if (c == "none")
      compression = COMPRESSION_NONE;
    else
      std::cerr << "TIFCodec: Unrecognized compression option '" << compress << "'" << std::endl;
  }
  
  if (page) {
    TIFFSetField (out, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField (out, TIFFTAG_PAGENUMBER, page, 0); // total number unknown
    // TIFFSetField (out, TIFFTAG_PAGENAME, page_name);
  }
  
  TIFFSetField (out, TIFFTAG_IMAGEWIDTH, image.w);
  TIFFSetField (out, TIFFTAG_IMAGELENGTH, image.h);
  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, image.bps);
  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, image.spp);
  TIFFSetField (out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  TIFFSetField (out, TIFFTAG_COMPRESSION, compression);
  if (image.spp == 1 && image.bps == 1)
    // internally we actually have MINISBLACK, but some programs,
    // including the Apple Preview.app appear to ignore this bit
    TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
  else if (image.spp == 1)
    TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  else if (false) { /* just saved for reference */
    uint16 rmap[256], gmap[256], bmap[256];
    for (int i = 0;i < 256; ++i) {
      rmap[i] = gmap[i] = bmap[i] = i * 0xffff / 255;
    }
    TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
    TIFFSetField (out, TIFFTAG_COLORMAP, rmap, gmap, bmap);
  }
  else
    TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  
  if (image.resolutionX() != 0) {
    float _xres = image.resolutionX();
    TIFFSetField (out, TIFFTAG_XRESOLUTION, _xres);
  }
  
  if (image.resolutionY() != 0) {
    float _yres = image.resolutionY();
    TIFFSetField (out, TIFFTAG_YRESOLUTION, _yres);
  }
  
  if (page <= 1) {
    TIFFSetField (out, TIFFTAG_SOFTWARE, "ExactImage");
  }
  //TIFFSetField (out, TIFFTAG_IMAGEDESCRIPTION, "");
  rowsperstrip = TIFFDefaultStripSize (out, rowsperstrip); 
  TIFFSetField (out, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
  
  const int stride = image.stride();
  /* Note: we on-the-fly invert 1-bit data to please some historic apps */
  
  uint8_t* src = image.getRawData();
  uint8_t* scanline = 0;
  if (image.bps == 1)
    scanline = (uint8_t*) malloc (stride);
  
  for (int row = 0; row < image.h; ++row, src += stride) {
    int err = 0;
    if (image.bps == 1) {
      for (int i = 0; i < stride; ++i)
        scanline [i] = src [i] ^ 0xFF;
      err = TIFFWriteScanline (out, scanline, row, 0);
    }
    else
      err = TIFFWriteScanline (out, src, row, 0);
    
    if (err < 0) {
      if (scanline) free (scanline);
      return false;
    }
  }
  if (scanline) free (scanline);  
  
  return TIFFWriteDirectory(out);
}

TIFCodec tif_loader;
