/*
 * Copyright (c) 2008 Valentin Ziegler <valentin@exactcode.de>
 * Copyright (c) 2008 Susanne Klaus <susanne@exactcode.de>
 * Copyright (c) 2009 Rene Rebe <rene@exactcode.de>
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


#include "eps.hh"
#include "ps.hh"

int EPSCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
    return false;
}

bool EPSCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
	// TODO: yres might be different
	double dpi = image.resolutionX() ? image.resolutionX() : 72;
	double scale = 72 / dpi; //postscript native resolution is 72dpi.

	*stream << 
		"%!PS-Adobe-3.0 EPSF-3.0\n"
		"%%BoundingBox: 0 0 " << scale*(double)image.w << " " << scale*(double)image.h << "\n"
		"0 dict begin"
	<< std::endl;
	
	PSCodec::encodeImage (stream, image, scale, quality, compress);

	*stream << "showpage\nend" << std::endl;

 	return true;
}

EPSCodec eps_loader;
