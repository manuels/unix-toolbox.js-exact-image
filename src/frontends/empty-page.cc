/*
 * Copyright (C) 2005-2010 René Rebe
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

#include <iostream>

#include "Image.hh"
#include "Codecs.hh"

#include "ArgumentList.hh"

#include "empty-page.hh"

using namespace Utility;

ArgumentList arglist;

bool usage(const Argument<bool>& arg)
{
  std::cerr << "Empty page detector"
            <<  " - Copyright 2005-2010 by René Rebe" << std::endl
            << "Usage:" << std::endl;
  
  arglist.Usage(std::cerr);
  exit(1);
}

int main (int argc, char* argv[])
{
  // setup the argument list
  Argument<bool> arg_help ("h", "help",
			   "display this help text and exit");
  Argument<std::string> arg_input ("i", "input", "input file",
                                   1, 1);
  Argument<int> arg_margin ("m", "margin",
			    "border margin to skip", 16, 0, 1);
  Argument<float> arg_percent ("p", "percentage",
			       "coverate for non-empty page", 0.05, 0, 1);
  
  arg_help.Bind (usage);
  
  arglist.Add (&arg_help);
  arglist.Add (&arg_input);
  arglist.Add (&arg_margin);
  arglist.Add (&arg_percent);
  
  // parse the specified argument list - and maybe output the Usage
  if (!arglist.Read (argc, argv))
    {
      usage(arg_help);
    }
  
  const int margin = arg_margin.Get();
  if (margin % 8 != 0) {
    std::cerr << "For speed reasons, the margin has to be a multiple of 8."
	      << std::endl;
    return 1;
  }
  
  Image image;
  if (!ImageCodec::Read (arg_input.Get(), image)) {
    std::cerr << "Error reading input file." << std::endl;
    return 1;
  }
  
  int set_pixels;
  const double percent = arg_percent.Get();
  bool is_empty = detect_empty_page (image, percent, margin, &set_pixels);
  
  double image_percent = (float) set_pixels / (image.w * image.h) * 100;
  std::cerr << "The image has " << set_pixels
	    << " dark pixels from a total of "
	    << image.w * image.h
	    << " (" << image_percent << "%)." << std::endl;
  
  if (!is_empty) {
    std::cerr << "non-empty" << std::endl;
    return 1;
  }
  
  std::cerr << "empty" << std::endl;
  return 0;
}
