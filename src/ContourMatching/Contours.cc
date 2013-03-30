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

#include <iostream>
#include <iomanip>

#include "ArgumentList.hh"
#include "Codecs.hh"
#include "Colorspace.hh"
#include "Matrix.hh"
#include "optimize2bw.hh"
#include "Contours.hh"

void PutPixel(Image& img, int x, int y, uint16_t  R, uint16_t G, uint16_t B)
{
  Image::iterator color=img.begin();
  color.setRGB(R, G, B);

  Image::iterator p=img.begin();
  p=p.at(x,y);
  p.set(color);
}

void DrawContour(Image& img, const Contours::Contour& c, uint16_t r, uint16_t g, uint16_t b)
{
  for (unsigned int i=0; i<c.size(); i++)
    PutPixel(img, c[i].first, c[i].second, r, g, b);
}


using namespace Utility;


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

  arglist.Add (&arg_help);
  arglist.Add (&arg_input);
  arglist.Add (&arg_output);
  arglist.Add (&arg_low);
  arglist.Add (&arg_high);
  arglist.Add (&arg_threshold);
  arglist.Add (&arg_radius);
  arglist.Add (&arg_sd);


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

  FGMatrix m(image, threshold);

  std::cout << "Contouring" << std::endl;
  Contours cont(m);
  std::cout << "done." << std::endl;

  for (unsigned int i=0; i<cont.contours.size(); i++) {
    unsigned int r;
    unsigned int g;
    unsigned int b;
    switch (i % 6) {
    case 0: r=255; g=0; b=0; break;
    case 1: r=0; g=255; b=0; break;
    case 2: r=0; g=0; b=255; break;
    case 3: r=255; g=255; b=0; break;
    case 4: r=0; g=255; b=255; break;
    case 5: r=255; g=0; b=255; break;
    }
    DrawContour(o_image, *cont.contours[i], r,g,b);
  }

  if (!ImageCodec::Write(arg_output.Get(), o_image)) {
    std::cerr << "Error writing output file." << std::endl;
    return 1;
  }
  return 0;
}
