/*
 * Copyright (C) 2007 - 2008 Valentin Ziegler, ExactCODE GmbH Germany.
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


#include "ArgumentList.hh"

#include "segmentation.hh"
#include "Codecs.hh"
#include "Colorspace.hh"
#include "Matrix.hh"

#include "optimize2bw.hh"

using namespace Utility;


void Draw(Segment* s, Image& img)
{
  if (s->children.size() == 0)
    s->Draw(img);
  else for (unsigned int i=0; i<s->children.size(); i++)
    Draw(s->children[i], img);
}

int main (int argc, char* argv[])
{
  ArgumentList arglist;
  
  // setup the argument list
  Argument<bool> arg_help ("", "help",
			   "display this help text and exit");
  Argument<std::string> arg_input ("i", "input", "input file",
                                   1, 1);
  Argument<std::string> arg_output ("o", "output", "output file",
				    1, 1);
  Argument<int> arg_low ("l", "low",
			 "low normalization value", 0, 0, 1);
  Argument<int> arg_high ("h", "high",
			  "high normalization value", 0, 0, 1);
  
  Argument<int> arg_threshold ("t", "threshold",
			       "bi-level threshold value", 0, 0, 1);

  Argument<int> arg_radius ("r", "radius",
			    "\"unsharp mask\" radius", 0, 0, 1);

  Argument<double> arg_sd ("sd", "standard-deviation",
			   "standard deviation for Gaussian distribution", 0.0, 0, 1);
  
  Argument<double> arg_tolerance ("T", "tolerance",
			       "fraction of foreground pixels tolerated per line", 0.02, 0, 1);

  Argument<int> arg_width("W", "width",
			  "minimum width of vertical separator", 10, 0, 1);

  Argument<int> arg_height("H", "height",
			   "minimum height of horizontal separator", 20, 0, 1);


  arglist.Add (&arg_help);
  arglist.Add (&arg_input);
  arglist.Add (&arg_output);
  arglist.Add (&arg_low);
  arglist.Add (&arg_high);
  arglist.Add (&arg_threshold);
  arglist.Add (&arg_radius);
  arglist.Add (&arg_sd);
  arglist.Add (&arg_tolerance);
  arglist.Add (&arg_width);
  arglist.Add (&arg_height);


  // parse the specified argument list - and maybe output the Usage
  if (!arglist.Read (argc, argv) || arg_help.Get() == true)
    {
      std::cerr << "Based on Color / Gray image to Bi-level optimizer"
                <<  " - Copyright 2005, 2006 by René Rebe" << std::endl
                << "Usage:" << std::endl;
      
      arglist.Usage (std::cerr);
      return 1;
    }

  Image o_image;
  if (!ImageCodec::Read (arg_input.Get(), o_image)) {
    std::cerr << "Error reading input file." << std::endl;
    return 1;
  }

  Image image=o_image;
  
  int low = 0;
  int high = 0;
  int sloppy_threshold = 0;
  int radius = 3;
  double sd = 2.1;
  
  if (arg_low.Get() != 0) {
    low = arg_low.Get();
    std::cerr << "Low value overwritten: " << low << std::endl;
  }
  
  if (arg_high.Get() != 0) {
    high = arg_high.Get();
    std::cerr << "High value overwritten: " << high << std::endl;
  }
  
  if (arg_radius.Get() != 0) {
    radius = arg_radius.Get();
    std::cerr << "Radius: " << radius << std::endl;
  }
  
  if (arg_sd.Get() != 0) {
    sd = arg_sd.Get();
    std::cerr << "SD overwritten: " << sd << std::endl;
  }
  
  // convert to 1-bit (threshold)
  int threshold = 0;
  
  if (arg_threshold.Get() != 0) {
    threshold = arg_threshold.Get();
    std::cerr << "Threshold: " << threshold << std::endl;
  }
  
  optimize2bw (image, low, high, threshold, sloppy_threshold, radius, sd);

  if (arg_threshold.Get() == 0)
    threshold = 200;

  double tolerance=arg_tolerance.Get();
  int width=arg_width.Get();
  int height=arg_height.Get();
  // TODO: check for nonsense values

  FGMatrix foreground(image, threshold);
  Segment* segment=segment_image(foreground, tolerance, width, height);
  segment = segment;
  Draw(segment, o_image);


  if (!ImageCodec::Write(arg_output.Get(), o_image)) {
    std::cerr << "Error writing output file." << std::endl;
    return 1;
  }
  return 0;
}
