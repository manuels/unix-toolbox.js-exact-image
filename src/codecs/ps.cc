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

#include "ps.hh"
#include "Encodings.hh"

#if WITHLIBJPEG == 1
#include "jpeg.hh"
#endif

int PSCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
    return false;
}

bool PSCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
	const int w = image.w;
	const int h = image.h;
	// TODO: yres might be different
	double dpi = image.resolutionX() ? image.resolutionY() : 72;
	double scale = 72 / dpi; //postscript native resolution is 72dpi.

	const char* creatorName = "ExactImage";

	*stream <<
		"%!PS-Adobe-3.0\n"
		"%%Creator:" << creatorName << "\n"
		"%%DocumentData: Clean7Bit\n"
		"%%LanguageLevel: 2\n"
		"%%BoundingBox: 0 0 " << scale*(double)w << " " << scale*(double)h << "\n"
		"%%EndComments\n"
		"%%BeginProlog\n"
		"0 dict begin\n"
		"%%EndProlog\n"
		"%%BeginPage\n"
	<< std::endl;

	encodeImage (stream, image, scale, quality, compress);

	*stream << "%%EndPage\nshowpage\nend" << std::endl;

 	return true;
}

void PSCodec::encodeImage (std::ostream* stream, Image& image, double scale,
			   int quality, const std::string& compress)
{
	const int w = image.w;
	const int h = image.h;

	std::string encoding = "ASCII85Decode";

	if (!compress.empty())
	{
		std::string c (compress);
		std::transform (c.begin(), c.end(), c.begin(), tolower);

		if (c == "encodeascii85")
			encoding = "ASCII85Decode";
		else if (c == "encodehex")
			encoding = "ASCIIHexDecode";
		else if (c == "encodejpeg")
			encoding = "DCTDecode";
		else
			std::cerr << "PDFCodec: Unrecognized encoding option '" << compress << "'" << std::endl;
	}

	const char* decodeName = "Decode [0 1 0 1 0 1]";
	const char* deviceName = "DeviceRGB";

 	if (image.spp == 1) {
		deviceName = "DeviceGray";
		decodeName = "Decode [0 1]";
	}

	*stream << 
		"/" << deviceName << " setcolorspace\n"
		"<<\n"
		"   /ImageType 1\n"
		"   /Width " << w << " /Height " << h << "\n"
		"   /BitsPerComponent " << image.bps << "\n"
		"   /" << decodeName << "\n"
		"   /ImageMatrix [\n"
		"       " << 1.0 / scale << " 0.0\n"
		"       0.0 " << -1.0 / scale << "\n"
		"       0.0 " << h << "\n"
		"   ]\n"
		"   /DataSource currentfile /" << encoding << " filter\n"
		">> image"
		<< std::endl;

	const int bytes = image.stride() * h;
	uint8_t* data = image.getRawData();
	if (encoding == "ASCII85Decode")
		EncodeASCII85(*stream, data, bytes);
	else if (encoding == "ASCIIHexDecode")
		EncodeHex(*stream, data, bytes);
#if WITHLIBJPEG == 1
	else {
		JPEGCodec codec;
		codec.writeImage (stream, image, quality, compress);
	}
#endif
	stream->put('\n');
}

PSCodec ps_loader;
