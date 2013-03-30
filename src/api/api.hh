/*
 * The ExactImage stable external API for use with SWIG.
 * Copyright (C) 2006 - 2009 René Rebe, ExactCODE GmbH
 * Copyright (C) 2006 - 2008 Archivista GmbH
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

/*
 * This header defines the public, supposedly stable, API that can
 * even be used from C as well as SWIG script language bindings.
 *
 * We decided to map the library internals in an non-OO (Object
 * Oriented) way in order to archive the most flexible external
 * language choice possible and not to hold of refactoring from
 * time to time.
 *
 * (People demanding a detailed and Object Oriented interface
 *  can still choose to use the fine grained intern C++ API
 *  [which however is not set in stone {at least not yet}].)
 */

#include <string>

#include "config.h"

class Image; // just forward, never ever care about the internal layout

// instanciate new image class
Image* newImage ();

// instanciate new imgae class with specified sample cound, bit-depth
// and width and height - if fill is 0 the image will be pre-filled
// with RGBA(0,0,0,0), if fill is 1 the current background color will
// be used.
Image* newImageWithTypeAndSize (unsigned int samplesPerPixel, // e.g. 3
				unsigned int bitsPerSample, // e.g. 8
				unsigned int width, unsigned int height, int fill = 0);

// destroy image instance
void deleteImage (Image* image);

// copy the image's pixel and meta data
Image* copyImage (Image* image);
Image* copyImageCropRotate (Image* image, int x, int y,
			   unsigned int w, unsigned int h, double angle);

// decode image from memory data of size n
#if defined(SWIG) && !defined(SWIG_CSTRING_UNIMPL)
%apply (char *STRING, int LENGTH) { (char *data, int n) };
#endif
#if !defined(SWIG) || (defined(SWIG) && !defined(SWIG_CSTRING_UNIMPL))
bool decodeImage (Image* image, char* data, int n);
#endif

#if !defined(SWIG) || (defined(SWIG) && defined(SWIG_CSTRING_UNIMPL))
bool decodeImage (Image* image, const std::string& data);
#endif

// decode image from given filename
bool decodeImageFile (Image* image, const char* filename);


// encode image to memory, the data is newly allocated and returned
// return 0 i the image could not be decoded

#if defined(SWIG) && !defined(SWIG_CSTRING_UNIMPL)
%cstring_output_allocate_size(char ** s, int *slen, free(*$1))
#endif
#if !defined(SWIG) || (defined(SWIG) && !defined(SWIG_CSTRING_UNIMPL))
void encodeImage (char **s, int *slen,
		  Image* image, const char* codec, int quality = 75,
		  const char* compression = "");
#endif
#if !defined(SWIG) || (defined(SWIG) && defined(SWIG_CSTRING_UNIMPL))
const std::string encodeImage (Image* image, const char* codec, int quality = 75,
                               const char* compression = "");
#endif

// encode image into specified filename
bool encodeImageFile (Image* image, const char* filename,
		      int quality = 75, const char* compression = "");


// image properties
int imageChannels (Image* image);
int imageChannelDepth (Image* image);

int imageWidth (Image* image);
int imageHeight (Image* image);

// returns the name of the image colorspace such as gray, gray2, gray4, rgb8, rgb16, cymk8, cymk16 ...
const char* imageColorspace (Image* image);

// returns X and Y resolution
int imageXres (Image* image);
int imageYres (Image* image);

// set X and Y resolution
void imageSetXres (Image* image, int xres);
void imageSetYres (Image* image, int yres);


// image manipulation

// single pixel access, attention: slow - just for testing / drafting
#ifdef SWIG
%apply double *OUTPUT { double* r, double* g, double* b, double* a };
#endif
void get(Image* image, unsigned int x, unsigned int y, double* r, double* g, double* b, double* a);
void set(Image* image, unsigned int x, unsigned int y, double r, double g, double b, double a = 1.0);

// transforms the image into a new target colorspace - the names are the same as returned by
// imageColorspace, might return false if the conversion was not possible
// the threshold is used while converting to black&white (1 bit) data
bool imageConvertColorspace (Image* image, const char* target_colorspace, int threshold = 127);

void imageResize (Image* image, int x, int y);
void imageRotate (Image* image, double angle);

void imageFlipX (Image* image);
void imageFlipY (Image* image);

// best scale (or thru the codec (e.g. JPEG)) or explicit algorithm
// if yfactor is not specified, the same factor is used for both directions
void imageScale (Image* image, double factor, double yfactor = .0);
void imageNearestScale (Image* image, double factor, double yfactor = .0);
void imageBoxScale (Image* image, double factor, double yfactor = .0);
void imageBilinearScale (Image* image, double factor, double yfactor = .0);
void imageThumbnailScale (Image* image, double factor, double yfactor = .0);

void imageCrop (Image* image, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

// fast auto crop by equal background color
// (currently only crops the bottom, might be expanded in the
//  future to allow top, left, right as well with always just
//  the bottom crop enabled by default)
void imageFastAutoCrop (Image* image);

// color controls

void setForegroundColor (double r, double g, double b, double a = 1.0);
void setBackgroundColor (double r, double g, double b, double a = 1.0);

void imageNormalize (Image* image);

void imageInvert (Image* image);
void imageBrightnessContrastGamma (Image* image, double brightness, double contrast, double gamma);
void imageHueSaturationLightness (Image* image, double hue, double saturation, double lightness);

// vector elements
void setLineWidth (double width);
void imageDrawLine (Image* image, double x, double y, double x2, double y2);
void imageDrawRectangle (Image* image, double x, double y, double x2, double y2);

class Path; // external path
Path* newPath();
void deletePath(Path* path);

void pathClear(Path* path); // removes existing path elements

void pathMoveTo(Path* path, double x, double y);
void pathLineTo(Path* path, double x, double y);
void pathCurveTo(Path* path, double x, double y, double x2, double y2);
void pathQuadCurveTo(Path* path, double x, double y, double x2, double y2, double x3, double y3);
void pathClose(Path* path);

void pathStroke(Path* path, Image* image);
void pathFill(Path* path, Image* image);

#if WITHFREETYPE == 1
void imageDrawText(Image* image, double x, double y, char* text,
		   double height, const char* fontfile = NULL);
void imageDrawTextOnPath(Image* image, Path* path, char* text,
			 double height, const char* fontfile = NULL);
#endif


// advanced all-in-one algorithms
void imageOptimize2BW (Image* image, int low = 0, int high = 255,
		       int threshold = 170,
		       int radius = 3, double standard_deviation = 2.3,
		       int target_dpi = 0);
// remeber: the margin will be rounded down to a multiple of 8, ...
bool imageIsEmpty (Image* image, double percent, int margin);


#if WITHBARDECODE == 1
// commercial bardecode
// codes is the string of barcode to look for, | seperated, like:
// CODE39|CODE128|CODE25|EAN13|EAN8|UPCA|UPCE
// case doesn't matter
// 
// returned is an alternating array of codes and types, like
// "1234", "EAN", "5678", "CODE128", ...
#ifdef SWIG
#ifdef SWIGPERL5
// Creates a new Perl array and places a NULL-terminated char ** into it
%typemap(out) char** {
  AV *myav;
  SV **svs;
  int i = 0,len = 0;
  /* Figure out how many elements we have */
  while ($1[len])
    len++;
  svs = (SV **) malloc(len*sizeof(SV *));
  for (i = 0; i < len ; i++) {
    svs[i] = sv_newmortal();
    sv_setpv((SV*)svs[i],$1[i]);
    free ($1[i]);
  };
  myav =  av_make(len,svs);
  free(svs);
  free($1);
  $result = newRV((SV*)myav);
  sv_2mortal($result);
  argvi++;
}
#endif
#endif
/* directions bitfield of directions to be scanned:
   1: left-to-right, 0 degree to image data
   2: top-down, 90 degree to image data
   4: right-to-left, 180 degree to image data
   8: down-to-top, -90 degree to image data */
char** imageDecodeBarcodesExt (Image* image, const char* codes,
			       unsigned int min_length = 0,
                               unsigned int max_length = 0, int multiple = 0, int directions = 0xf);
#endif

#ifdef SWIG
#ifdef SWIGPERL5
// Creates a new Perl array and places a NULL-terminated char ** into it
%typemap(out) char** {
  AV *myav;
  SV **svs;
  int i = 0,len = 0;
  /* Figure out how many elements we have */
  while ($1[len])
    len++;
  svs = (SV **) malloc(len*sizeof(SV *));
  for (i = 0; i < len ; i++) {
    svs[i] = sv_newmortal();
    sv_setpv((SV*)svs[i],$1[i]);
    free ($1[i]);
  };
  myav =  av_make(len,svs);
  free(svs);
  free($1);
  $result = newRV((SV*)myav);
  sv_2mortal($result);
  argvi++;
}
#endif
#ifdef SWIGLUA
// Creates a new Lua table and places a NULL-terminated char ** into it
%typemap(out) char** {
  lua_newtable(L);
  int i = 0,len = 0;
  /* Figure out how many elements we have */
  while ($1[len])
    len++;
  for (i = 0; i < len ; i++) {
    lua_pushnumber(L, 1.+i);
    lua_pushstring(L, $1[i]);
    lua_settable(L, -3);
    free ($1[i]);
  };
  free($1);
  return 1;
}
#endif

#endif
/* directions bitfield of directions to be scanned:
   1: left-to-right, 0 degree to image data
   2: top-down, 90 degree to image data
   4: right-to-left, 180 degree to image data
   8: down-to-top, -90 degree to image data */
char** imageDecodeBarcodes (Image* image, const char* codes,
			    unsigned int min_length = 0,
                            unsigned int max_length = 0,
                            int multiple = 0, unsigned int line_skip = 8, int directions = 0xf);

/* contour matching functions
 * attention:
 * this part of the api is in an evaluation phase and not yet written in stone !!
 */

class Contours;
class LogoRepresentation;

Contours* newContours(Image* image, int low = 0, int high = 0,
		       int threshold = 0,
		       int radius = 3, double standard_deviation = 2.1);


void deleteContours(Contours* contours);

LogoRepresentation* newRepresentation(Contours* logo_contours,
			    int max_feature_no=10,
			    int max_avg_tolerance=20,
			    int reduction_shift=3,
			    double maximum_angle=0.0,
			    double angle_step=0.0);

void deleteRepresentation(LogoRepresentation* representation);

double matchingScore(LogoRepresentation* representation, Contours* image_contours);

// theese are valid after call to MatchingScore()
double logoAngle(LogoRepresentation* representation);
int logoTranslationX(LogoRepresentation* representation);
int logoTranslationY(LogoRepresentation* representation);

int inverseLogoTranslationX(LogoRepresentation* representation, Image* image);
int inverseLogoTranslationY(LogoRepresentation* representation, Image* image);

void drawMatchedContours(LogoRepresentation* representation, Image* image);
