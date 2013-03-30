/*
 * Copyright (C) 2005 - 2011 René Rebe
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

#include "Codecs.hh"

#include <tiffconf.h>
#include <tiffio.h>

// not TIFFCodec due the TIFFCodec in libtiff itself ...
class TIFCodec : public ImageCodec {
public:
  
  TIFCodec ();
  TIFCodec (TIFF* ctx);
  ~TIFCodec ();
  virtual std::string getID () { return "TIFF"; };
  
  virtual int readImage (std::istream* stream, Image& image, const std::string& decompres, int index);
  virtual bool writeImage (std::ostream* stream, Image& image,
			   int quality, const std::string& compress);

  // for multi-page writing
  virtual ImageCodec* instanciateForWrite (std::ostream* stream);
  virtual bool Write (Image& image,
		      int quality, const std::string& compress, int index);
  
private:
  
  static bool writeImageImpl (TIFF* out, const Image& image, const std::string& conpress, int page = 0);

private:
  TIFF* tiffCtx;
};
