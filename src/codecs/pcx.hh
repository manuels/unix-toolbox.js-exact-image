/*
 * Copyright (C) 2008 René Rebe
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

class PCXCodec : public ImageCodec {
public:
  
  PCXCodec () { registerCodec ("pcx", this); };
  
  virtual std::string getID () { return "PCX"; };

  virtual int readImage (std::istream* stream, Image& image, const std::string& decompress);
  virtual bool writeImage (std::ostream* stream, Image& image, int quality, const std::string& compress);
};
