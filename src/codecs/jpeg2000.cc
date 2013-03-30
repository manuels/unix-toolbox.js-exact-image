/*
 * Copyright (C) 2005 - 2008 René Rebe
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

#include <iostream>
#include <sstream>

#include <jasper/jasper.h>
#include <jasper/jas_image.h>


#include "utils.hh"

#include "jpeg2000.hh"

static void add_color_prof(jas_image_t* image)
{
  /* Create a color profile if needed. */
  if (!jas_clrspc_isunknown(image->clrspc_) &&
      !jas_clrspc_isgeneric(image->clrspc_) && !image->cmprof_) {
    if (!(image->cmprof_ =
      jas_cmprof_createfromclrspc(jas_image_clrspc(image))))
      {
        std::cerr << "error: cannot create the colorspace" << std::endl;
        return;
      }
  }
}

static jas_stream_t* jas_stream_create()
{
  jas_stream_t* stream;
  
  if (!(stream = (jas_stream_t*) jas_malloc(sizeof(jas_stream_t)))) {
    return 0;
  }
  stream->openmode_ = 0;
  stream->bufmode_ = 0;
  stream->flags_ = 0;
  stream->bufbase_ = 0;
  stream->bufstart_ = 0;
  stream->bufsize_ = 0;
  stream->ptr_ = 0;
  stream->cnt_ = 0;
  stream->ops_ = 0;
  stream->obj_ = 0;
  stream->rwcnt_ = 0;
  stream->rwlimit_ = -1;
  
  return stream;
}

static int cpp_jas_read (jas_stream_obj_t* obj, char* buf, int cnt)
{
  std::istream* stream = (std::istream*) obj;
  stream->read (buf, cnt);
  return cnt;
}

static int cpp_jas_write (jas_stream_obj_t* obj, char* buf, int cnt)
{
  std::ostream* stream = (std::ostream*) obj;
  stream->write (buf, cnt);
  return cnt;
}

static long cpp_jas_seek (jas_stream_obj_t* obj, long offset, int origin)
{
  std::cerr << __FUNCTION__ << " implement me ,-)" << std::endl;
  /*
  std::istream* stream = (std::istream*) obj;
  stream->read (buf, cnt);
  return cnt;
  */
  return 0;
}

static int cpp_jas_close (jas_stream_obj_t* obj)
{
  // NOP, nothing to do
  return 0;
}

static jas_stream_ops_t cpp_jas_stream_ops = {
  cpp_jas_read,
  cpp_jas_write,
  cpp_jas_seek,
  cpp_jas_close
};

static void jas_stream_initbuf (jas_stream_t* stream)
{
  stream->bufbase_ = (uint8_t*) jas_malloc (JAS_STREAM_BUFSIZE + JAS_STREAM_MAXPUTBACK);
  if (stream->bufbase_) {
    stream->bufmode_ |= JAS_STREAM_FREEBUF;
    stream->bufsize_ = JAS_STREAM_BUFSIZE;
  }
  else {
    stream->bufbase_ = stream->tinybuf_;
    stream->bufsize_ = 1;
  }
  
  stream->bufstart_ = &stream->bufbase_ [JAS_STREAM_MAXPUTBACK];
  stream->ptr_ = stream->bufstart_;
  stream->cnt_ = 0;
  stream->bufmode_ |= JAS_STREAM_BUFMODEMASK;
}

int JPEG2000Codec::readImage (std::istream* stream, Image& im, const std::string& decompres)
{
  {
    // quick magic check
    char buf [6];
    stream->read (buf, sizeof (buf));
    stream->seekg (0);
    if (buf[4] != 'j' || buf[5] != 'P')
      return false;
  }
  
  jas_image_t* image;
  jas_stream_t* in;
  
  if (!(in = jas_stream_create ())) {
    std::cerr << "error: cannot create stream" << std::endl;
    return false;
  }
  
  // fill stream details
  in->openmode_ = JAS_STREAM_BINARY | JAS_STREAM_READ;
  in->obj_ = stream;
  in->ops_ = &cpp_jas_stream_ops;
  
  jas_stream_initbuf (in);
  
  if (!(image = jp2_decode(in, 0))) {
    std::cerr << "error: cannot load image data" << std::endl;
    return false;
  }
  
  add_color_prof(image);

  jas_stream_close (in);

  im.w = jas_image_width (image);
  im.h = jas_image_height (image);

#define PRINT(a,b) case a: std::cout << "Clrspc: " << a << ", " << b << std::endl; break;

  switch (jas_image_clrspc(image)) {
    PRINT(JAS_CLRSPC_UNKNOWN, "UNKNOWN")
    PRINT(JAS_CLRSPC_CIEXYZ, "CIEXYZ")
    PRINT(JAS_CLRSPC_CIELAB, "CIELAB")
    PRINT(JAS_CLRSPC_SGRAY, "SGRAY")
    PRINT(JAS_CLRSPC_SRGB, "SRGB")
    PRINT(JAS_CLRSPC_SYCBCR, "SYCBCR")
    PRINT(JAS_CLRSPC_GENGRAY, "GENRGB")
    PRINT(JAS_CLRSPC_GENRGB, "GENRGB")
    PRINT(JAS_CLRSPC_GENYCBCR, "GENYCBCR")
    default:
      std::cerr << "Yet unknown colorspace ..." << std::endl;
  }

  // convert colorspace
  switch (jas_image_clrspc(image)) {
    case JAS_CLRSPC_SGRAY:
    case JAS_CLRSPC_SRGB:
    case JAS_CLRSPC_GENGRAY:
    case JAS_CLRSPC_GENRGB:
      break;
    default:
      {
        jas_image_t *newimage;
        jas_cmprof_t *outprof;

        std::cerr << "forcing conversion to sRGB" << std::endl;
        if (!(outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB))) {
          std::cerr << "cannot create sRGB profile" << std::endl;
          return 0;
        }
        std::cerr << "in space: " << jas_image_cmprof(image) << std::endl;
        if (!(newimage = jas_image_chclrspc(image, outprof, JAS_CMXFORM_INTENT_PER))) {
          std::cerr << "cannot convert to sRGB" << std::endl;
          return 0;
        }

        jas_image_destroy(image);
        jas_cmprof_destroy(outprof);
        image = newimage;
	std::cerr << "converted to sRGB" << std::endl;
     }
  }

  im.spp = jas_image_numcmpts(image);
  im.bps = jas_image_cmptprec(image, 0/*component*/);
  if (im.bps != 1 && im.bps != 8) // we do not support the others, yet
	im.bps = 8;

  std::cerr << "Components: " << jas_image_numcmpts(image)
            << ", precision: " << jas_image_cmptprec(image, 0) << std::endl;

  im.resize (im.w, im.h);
  uint8_t* data = im.getRawData ();
  uint8_t* data_ptr = data;

  jas_matrix_t *jasdata[3];
  for (int k = 0; k < im.spp; ++k) {
    if (!(jasdata[k] = jas_matrix_create(im.h, im.w))) {
      std::cerr << "internal error" << std::endl;;
      return 0;
    }

    if (jas_image_readcmpt(image, k, 0, 0, im.w, im.h, jasdata[k])) {
      std::cerr << "cannot read component data " << k << std::endl;
      return 0;
    }
  }

  int v [3];
  for (int y = 0; y < im.h; ++y) {
    for (int x = 0; x < im.w; ++x) {
       for (int k = 0; k < im.spp; ++k) {
         v[k] = jas_matrix_get (jasdata[k], y, x);
         // if the precision of the component is not supported, scale it
         int prec = jas_image_cmptprec(image, k);
	 if (prec < 8)
           v[k] <<= 8 - prec;
	 else
	   v[k] >>= prec - 8;
       }

       for (int k = 0; k < im.spp; ++k)
	 *data_ptr++ = v[k];
    }
  }
  
  jas_image_destroy (image);
  return true;
}


bool JPEG2000Codec::writeImage (std::ostream* stream, Image& im, int quality,
				const std::string& compress)
{
  jas_image_t* image;
  jas_stream_t* out;
  
  if (!(out = jas_stream_create ())) {
    std::cerr << "error: cannot create stream" << std::endl;
    return false;
  }
  
  // fill stream details
  out->openmode_ = JAS_STREAM_BINARY | JAS_STREAM_WRITE;
  out->obj_ = stream;
  out->ops_ = &cpp_jas_stream_ops;
  
  jas_stream_initbuf (out);
  
  jas_image_cmptparm_t compparms[3];

  for (int i = 0; i < im.spp; ++i) {
    compparms[i].tlx = 0;
    compparms[i].tly = 0;
    compparms[i].hstep = 1;
    compparms[i].vstep = 1;
    compparms[i].width = im.w;
    compparms[i].height = im.h;
    compparms[i].prec = im.bps;
    compparms[i].sgnd = false;
  }
  
  if (!(image = jas_image_create(im.spp, compparms,
                                 im.spp==3?JAS_CLRSPC_SRGB:JAS_CLRSPC_SGRAY))) {
    std::cerr << "error creating jasper image" << std::endl;
  }

  jas_matrix_t *jasdata[3];
  for (int i = 0; i < im.spp; ++i) {
    if (!(jasdata[i] = jas_matrix_create(im.h, im.w))) {
      std::cerr << "internal error" << std::endl;
      return false;
    }
  }

  uint8_t* it = im.getRawData();
  for (int y = 0; y < im.h; ++y) {
    for (int x = 0; x < im.w; ++x) {
      for (int k = 0; k < im.spp; ++k)
        jas_matrix_set(jasdata[k], y, x, *it++);
    }
  }

  for (int i = 0; i < im.spp; ++i) {
    int ct = JAS_IMAGE_CT_GRAY_Y;
    if (im.spp > 1)
      switch (i) {
       case 0: ct = JAS_IMAGE_CT_RGB_R; break;
       case 1: ct = JAS_IMAGE_CT_RGB_G; break;
       case 2: ct = JAS_IMAGE_CT_RGB_B; break;
    }
    jas_image_setcmpttype (image, i, ct );
    if (jas_image_writecmpt(image, i, 0, 0, im.w, im.h, jasdata[i])) {
      std::cerr << "error writing converted data into jasper" << std::endl;
      return false;
    }
  }

  std::stringstream opts;
  opts << "rate=" << (double)quality / 100;
  jp2_encode(image, out, (char*)opts.str().c_str());
  jas_image_destroy (image);
  jas_stream_close (out);
  
  return true;
}

JPEG2000Codec jpeg2000_loader;
