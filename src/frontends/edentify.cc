/*
 * The ExactImage library's identify compatible command line frontend.
 * Copyright (C) 2006 - 2010 René Rebe
 * Copyright (C) 2006 Archivista
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
#include <iomanip>

#include "config.h"

#include "ArgumentList.hh"
#include "File.hh"

#include "Image.hh"
#include "Codecs.hh"

#include "Colorspace.hh"

#include "scale.hh"
#include "rotate.hh"
#include "Matrix.hh"
#include "riemersma.h"
#include "floyd-steinberg.h"

#include <functional>

using namespace Utility;

Image image; // the global Image we work on

int main (int argc, char* argv[])
{
  ArgumentList arglist (true); // enable residual gathering
  
  // setup the argument list
  Argument<bool> arg_help ("h", "help",
			   "display this help text and exit");
  arglist.Add (&arg_help);

  Argument<bool> arg_verbose ("v", "verbose",
			      "more verbose output");
  arglist.Add (&arg_verbose);

  Argument<std::string> arg_format ("f", "format",
				    "user defined format string",
				    0, 1);
  arglist.Add (&arg_format);
  
  // parse the specified argument list - and maybe output the Usage
  if (!arglist.Read (argc, argv))
    return 1;

  if (arg_help.Get() == true || arglist.Residuals().empty())
    {
      std::cerr << "Exact image identification (edentify)."
		<< std::endl << "Version " VERSION
                <<  " - Copyright (C) 2006 - 2007 by ExactCODE and Archivista" << std::endl
                << "Usage:" << std::endl;
      
      arglist.Usage (std::cerr);
      return 1;
    }
  
  // for all residual arguments (image files)
  int errors = 0;
  const std::vector<std::string>& list = arglist.Residuals();
  for (std::vector<std::string>::const_iterator file = list.begin();
       file != list.end(); ++file) {
    for (int i = 0, n = 1; i < n; ++i)
    {
      int ret = ImageCodec::Read(*file, image, "", i);
      if (ret < 1) {
	std::cout << "edentify: unable to open image '" << *file << "'." << std::endl;
	continue;
      }
      if (i == 0) n = ret;
      
      if (arg_format.Size() > 0)
	{
	  const std::string& format = arg_format.Get();
	  
	  for (std::string::const_iterator it = format.begin(); it != format.end(); ++it)
	    {
	      if (*it == '%') {
		if (++it == format.end())
		  --it; // print the % if at the end
		switch (*it) {
		  // case 'b': break; //   file size
		  // case 'c': break; //   comment
		case 'd': //   directory
		case 'e': //   filename extension
		case 'f': //   filename
		case 't': //   top of filename
		  {
		    Utility::File f (*file);
		    switch (*it) {
		    case 'd': std::cout << f.Dirname(); break;
		    case 'e': std::cout << f.Extension(); break;
		    case 'f': std::cout << f.Basename(); break;
		    case 't': std::cout << f.BasenameWOExtension(); break;
		    }
		  }
		  break;
		  // %g   page geometry
		case 'h': //  height
		  std::cout << image.h; break;
		case 'i': //   input filename
		  std::cout << *file; break;
		  //case 'k': break; //   number of unique colors
		  // %l   label
		  // %m   magick
		  // %n   number of scenes
		  // %o   output filename
		  // %p   page number
		case 'q': //   quantum depth
		  std::cout << image.bps; break; // TODO: report file container?
		  break;
		  // %r   image class and colorspace
		  // %s   scene number
		  // %u   unique temporary filename
		case 'w': //   width
		  std::cout << image.w; break;
		case 'x': //   x resolution
		  std::cout << image.resolutionX() << " PixelsPerInch"; break;
		case 'y': //   y resolution
		  std::cout << image.resolutionY() << " PixelsPerInch"; break;
		case 'z': //   image depth
		  std::cout << image.bps; break;
		  // %D   image dispose method
		  // %O   page offset
		case 'P': //   page width and height
		  std::cout << image.w << "x" << image.h; break;
		  // %Q   image compression quality
		  // %T   image delay
		  //  %@   bounding box
		  // %#   signature
		case '%':
		  std::cout << *it; break;
		default:
		  if (it != format.begin())
		    --it;
		  std::cout << *it;
		}
	      }
	      else if (*it == '\\')
		{
		  if (++it == format.end())
		    --it; // print the \ if at the end
		  switch (*it) {
		  case 'n': std::cout << std::endl; break;
		  case 't': std::cout << "\t"; break;
		  case 'r': std::cout << "\r"; break;
		  case '\\': std::cout << *it; break;
		  default:
		    if (it != format.begin())
		      --it;
		    std::cout << *it;
		  }
		}
	      else
		std::cout << *it;
	    }
	}
      else if (arg_verbose.Get())
	{
	  std::cout << "TODO: implement verbose output" << std::endl;
	}
      else {
	std::cout << *file;
	if (n > 1) std::cout << "[" << i << "]";
	std::cout << ": "
		  << (image.getDecoderID().empty() ? "NONE" : image.getDecoderID() )
		  << " " << image.w << "x" << image.h;
	
	if (image.resolutionY() && image.resolutionY())
	  std::cout << " @ " << image.resolutionX()
		    << "x" << image.resolutionY() << "dpi ("
		    << 254 * image.w / image.resolutionX() / 10 << "x"
		    << 254 * image.h / image.resolutionY() / 10 << "mm)";
	
	int bits = image.bps * image.spp;
	std::cout << " " << bits << " bit" << (bits>1 ? "s" : "") << ", "
		  << image.spp << " channel" << (image.spp>1 ? "s" : "") << std::endl;
	
	std::cout << std::endl;
      }
    }
  }
  return errors;
}
