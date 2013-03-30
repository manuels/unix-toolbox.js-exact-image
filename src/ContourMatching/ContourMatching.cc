#include <math.h>

#include <iostream>
#include <iomanip>

#include "ArgumentList.hh"
#include "Codecs.hh"
#include "Colorspace.hh"
#include "Matrix.hh"
#include "optimize2bw.hh"

#include "ContourMatching.hh"


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

  // optimize2bw options

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


  // matching options

  Argument<std::string> arg_logo ("L", "logo", "logo file",
                                   1, 1);

  Argument<unsigned int> arg_features("F", "features", "maximum number of logo features",
				      (unsigned int)10, 0, 1, false, false);

  Argument<unsigned int> arg_tolerance("T", "tolerance", "tolerated maximum average distance",
				       (unsigned int)20, 0, 1, false, false);


  Argument<double> arg_angle("A", "angle", "maximum rotation angle for pre-matching",
			     0.0, 0, 1);

  Argument<double> arg_step("S", "step", "rotation angle increment for pre-matching",
			    0.0, 0, 1);

  Argument<unsigned int> arg_shift("R", "reduction", "coordinate bit reduction for pre-matching",
				   (unsigned int)3, 0, 1, false, false);



  

  arglist.Add (&arg_help);
  arglist.Add (&arg_input);
  arglist.Add (&arg_logo);
  arglist.Add (&arg_output);
  arglist.Add (&arg_low);
  arglist.Add (&arg_high);
  arglist.Add (&arg_threshold);
  arglist.Add (&arg_radius);
  arglist.Add (&arg_sd);
  arglist.Add (&arg_features);
  arglist.Add (&arg_tolerance);
  arglist.Add (&arg_angle);
  arglist.Add (&arg_step);
  arglist.Add (&arg_shift);


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

  Image l_image;
  if (!ImageCodec::Read (arg_logo.Get(), l_image)) {
    std::cerr << "Error reading logo file." << std::endl;
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
  optimize2bw (l_image, low, high, threshold, sloppy_threshold, radius, sd);

  if (arg_threshold.Get() == 0)
    threshold = 200;

  FGMatrix mi(image, threshold);
  FGMatrix ml(l_image, threshold);

  std::cout << "Contouring" << std::endl;
  Contours conti(mi);
  Contours contl(ml);
  std::cout << "done." << std::endl;


#if false // Export test
  FILE* test=fopen("test.cnt", "w");
  if (!WriteContourArray(test, conti.contours))
    std::cout << "write error" << std::endl;
  fclose(test);

  
  Contours copy;
  test=fopen("test.cnt", "r");
  if (ReadContourArray(test, copy.contours)) {
    if (copy.contours.size() != conti.contours.size()) {
      std::cout << "contour count differs" << std::endl;
    } else {
      for (unsigned int c=0; c<copy.contours.size(); c++) {
	if (copy.contours[c]->size() != conti.contours[c]->size())
	  std::cout << "size" << std::endl;
	else 
	  for (unsigned int i=0; i<copy.contours[c]->size(); i++)
	    if ((*(copy.contours[c]))[i] != (*(conti.contours[c]))[i])
	      std::cout << "content\t" << i 
			<< "\t" << (*(copy.contours[c]))[i].first
			<< "\t" << (*(copy.contours[c]))[i].second
			<< "\t" << (*(conti.contours[c]))[i].first
			<< "\t" << (*(conti.contours[c]))[i].second
			<<  std::endl;
      }
    }
  } else
    std::cout << "read error" << std::endl;
  fclose(test);
#endif

  // Todo: check for insane values
  unsigned int features=arg_features.Get();
  unsigned int tolerance=arg_tolerance.Get();
  unsigned int shift=arg_shift.Get();
  double max_angle=arg_angle.Get();
  double angle_step=arg_step.Get();

  LogoRepresentation lrep(&contl, features, tolerance, shift, max_angle, angle_step);
  std::cout << "score: " << lrep.Score(&conti) << std::endl;
  int tx=lrep.logo_translation.first;
  int ty=lrep.logo_translation.second;
  double angle=M_PI * lrep.rot_angle / 180.0;
  std::cout << "logo translation: " << tx << "\t" << ty << "\trotation angle: " << lrep.rot_angle << std::endl;

  for (unsigned int i=0; i<lrep.mapping.size(); i++) {
    double trash;
    Contours::Contour transformed;
    RotCenterAndReduce(*lrep.mapping[i].first, transformed, angle, 0, 0, trash, trash);
    DrawTContour(o_image, transformed, tx, ty, 0, 0, 255);
    //transformed.clear();
    //RotCenterAndReduce(*lrep.mapping[i].first, transformed, 0, 0, 0, trash, trash);
    //DrawTContour(o_image, transformed, tx, ty, 0, 255, 255);
    DrawContour(o_image, *lrep.mapping[i].second, 0,  255, 0);
  }


  if (!ImageCodec::Write(arg_output.Get(), o_image)) {
    std::cerr << "Error writing output file." << std::endl;
    return 1;
  }
  return 0;
}
