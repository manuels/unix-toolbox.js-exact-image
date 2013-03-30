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

#include <math.h>

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <sstream>

#include <Image.hh>
#include <Codecs.hh>

#include <rotate.hh>
#include <scale.hh>
#include <crop.hh>

#include <Colorspace.hh>

#include <optimize2bw.hh>
#include <empty-page.hh>
#include <ContourMatching.hh>

#include "Tokenizer.hh"	// barcode decoding
#include "Scanner.hh"

#include <vectorial.hh>

#include "api.hh"


// initializer

static Image::iterator background_color;
static Image::iterator foreground_color;

struct it_color_init
{
  it_color_init (Image::iterator& it, double r, double g, double b, double a)
  {
     it.type = Image::RGB8A;
     it.setRGBA(r, g, b, a);
  }
};
static it_color_init bg_color_init (background_color, 0, 0, 0, 1);
static it_color_init fg_color_init (foreground_color, 1, 1, 1, 1);


Image* newImage ()
{
  return new Image;
}
Image* newImageWithTypeAndSize (unsigned int samplesPerPixel, unsigned int bitsPerSample,
				unsigned int width, unsigned int height, int fill)
{
  Image* image = newImage();
  image->spp = samplesPerPixel;
  image->bps = bitsPerSample;
  image->resize(width, height);
  
  // make sure it's clean initially, also because we only allow painting
  // OVER and thus not allowing to create transparency
  if (fill == 0)
    memset(image->getRawData(), 0, image->stride() * image->h);
  else {
    double r = 0, g = 0, b = 0, a = 0;
    background_color.getRGBA(r, g, b, a);
    
    Image::iterator it = image->begin();
    // optimization: only set FP based values once, copy the rest
    it.setRGBA(r, g, b, a);
    for (Image::iterator it_end = image->end(); it != it_end; ++it)
      it.set(it);
  }
  
  return image;
}

void deleteImage (Image* image)
{
  delete image;
}

Image* copyImage (Image* other)
{
  Image* image = new Image;
  *image = *other;
  return image;
}

bool decodeImage (Image* image, const std::string& data)
{
  std::istringstream stream (data);

  return ImageCodec::Read (&stream, *image);
}

bool decodeImage (Image* image, char* data, int n)
{
  const std::string str (data, n); 
  
  return decodeImage (image, str);
}

bool decodeImageFile (Image* image, const char* filename)
{
  return ImageCodec::Read (filename, *image);
}

void encodeImage (char **s, int *slen,
		  Image* image, const char* codec, int quality,
		  const char* compression)
{
  std::ostringstream stream (""); // empty string to start with
  
  ImageCodec::Write (&stream, *image, codec, "", quality, compression);
  stream.flush();
  
  char* payload = (char*) malloc (stream.str().size());
  
  memcpy (payload, stream.str().c_str(), stream.str().size());
  
  *s = payload;
  *slen = stream.str().size();
}

const std::string encodeImage (Image* image, const char* codec, int quality,
			       const char* compression)
{
  std::ostringstream stream (""); // empty string to start with

  ImageCodec::Write (&stream, *image, codec, "", quality, compression);
  stream.flush();

  return stream.str();
}

bool encodeImageFile (Image* image, const char* filename,
		      int quality, const char* compression)
{
  return ImageCodec::Write (filename, *image, quality, compression);
}


// image properties
int imageChannels (Image* image)
{
  return image->spp;
}

int imageChannelDepth (Image* image)
{
  return image->bps;
}

int imageWidth (Image* image)
{
  return image->w;
}

int imageHeight (Image* image)
{
  return image->h;
}

int imageXres (Image* image)
{
  return image->resolutionX();
}

int imageYres (Image* image)
{
  return image->resolutionY();
}

const char* imageColorspace (Image* image)
{
  return colorspace_name (*image);
}

void imageSetXres (Image* image, int xres)
{
  image->setResolutionX(xres);
}

void imageSetYres (Image* image, int yres)
{
  image->setResolutionY(yres);
}

// image manipulation

void get(Image* image, unsigned int x, unsigned int y, double* r, double* g, double* b, double* a)
{
  Image::iterator it = image->begin();
  it = it.at(x, y);
  *it;
  it.getRGBA(*r, *g, *b, *a);
}

void set(Image* image, unsigned int x, unsigned int y, double r, double g, double b, double a)
{
  Image::iterator it = image->begin();
  it = it.at(x, y);
  it.setRGBA(r, g, b, a);
  it.set(it);
  
  // TODO: this should not be done so frequently, for every pixel
  image->setRawData();
}

bool imageConvertColorspace (Image* image, const char* target_colorspace, int threshold)
{
  return colorspace_by_name (*image, target_colorspace, threshold);
}

void imageResize (Image* image, int x, int y)
{
  if (x < 0)
    x = 0;
  if (y < 0)
    y = 0;

  image->resize (x, y);
}

void imageRotate (Image* image, double angle)
{
  rotate (*image, angle, background_color);
}

Image* copyImageCropRotate (Image* image, int x, int y,
			   unsigned int w, unsigned int h, double angle)
{
  return copy_crop_rotate (*image, x, y, w, h, angle, background_color);
}

void imageFlipX (Image* image)
{
  flipX (*image);
}

void imageFlipY (Image* image)
{
  flipY (*image);
}

void imageScale (Image* image, double factor, double yfactor)
{
  scale (*image, factor, yfactor != .0 ? yfactor : factor);
}

void imageBoxScale (Image* image, double factor, double yfactor)
{
  box_scale (*image, factor, yfactor != .0 ? yfactor : factor);
}

void imageNearestScale (Image* image, double factor, double yfactor)
{
  nearest_scale (*image, factor, yfactor != .0 ? yfactor : factor);
}

void imageBilinearScale (Image* image, double factor, double yfactor)
{
  bilinear_scale (*image, factor, yfactor != .0 ? yfactor : factor);
}

void imageThumbnailScale (Image* image, double factor, double yfactor)
{
  thumbnail_scale (*image, factor, yfactor != .0 ? yfactor : factor);
}

void imageCrop (Image* image, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  crop (*image, x, y, w, h);
}

void imageFastAutoCrop (Image* image)
{
  fastAutoCrop (*image);
}

// color controls

void setForegroundColor (double r, double g, double b, double a)
{
  foreground_color.setRGBA(r, g, b, a);
}

void setBackgroundColor (double r, double g, double b, double a)
{
  background_color.setRGBA(r, g, b, a);
}

// vector elements

class drawStyle
{
public:
  drawStyle ()
    : width (1) {
  }
  
  double width;
  std::vector <double> dash;
} style;

void setLineWidth (double width)
{
  style.width = width;
}

void color_to_path (Path& p)
{
  double r = 0, g = 0, b = 0, a = 0;
  foreground_color.getRGBA (r, g, b, a);
  p.setFillColor (r, g, b, a);
}

void imageDrawLine (Image* image, double x, double y, double x2, double y2)
{
  Path path;
  path.moveTo (x, y);
  path.addLineTo (x2, y2);

  path.setLineWidth (style.width);
  path.setLineDash (0, style.dash);
  
  color_to_path(path);
  path.draw (*image);
}

void imageDrawRectangle (Image* image, double x, double y, double x2, double y2)
{
  Path path;
  path.addRect (x, y, x2, y2);
  path.setLineWidth (style.width);
  path.setLineDash (0, style.dash);
  path.setLineJoin (agg::miter_join);
  
  color_to_path(path);
  path.draw (*image);
}

#if WITHFREETYPE == 1
void imageDrawText (Image* image, double x, double y, char* text,
                    double height, const char* fontfile)
{
  Path path;
  
  color_to_path(path);
  path.moveTo (x, y);
  path.drawText (*image, text, height, fontfile);
}

void imageDrawTextOnPath (Image* image, Path* path, char* text,
			  double height, const char* fontfile)
{
  color_to_path(*path);
  path->drawTextOnPath (*image, text, height, fontfile);
}
#endif

Path* newPath()
{
  return new Path;
}
void deletePath(Path* path)
{
  delete path;
}

void pathClear(Path* path)
{
  path->clear();
}

void pathMoveTo(Path* path, double x, double y)
{
  path->moveTo(x, y);
}

void pathLineTo(Path* path, double x, double y)
{
  path->addLineTo(x, y);
}

void pathCurveTo(Path* path, double x, double y, double x2, double y2)
{
  path->addCurveTo(x, y, x2, y2);
}

void pathQuadCurveTo(Path* path, double x, double y, double x2, double y2, double x3, double y3)
{
  path->addCurveTo(x, y, x2, y2, x3, y3);
}

void pathClose(Path* path)
{
  path->close();
}

void pathStroke(Path* path, Image* image)
{
  color_to_path(*path);
  path->setLineWidth (style.width);
  path->draw(*image, Path::fill_none);
}

void pathFill(Path* path, Image* image)
{
  color_to_path(*path);
  path->draw(*image, Path::fill_non_zero);
}

void imageOptimize2BW (Image* image, int low, int high,
		       int threshold,
		       int radius, double sd, int target_dpi)
{
  optimize2bw (*image, low, high, threshold, 0 /* sloppy thr */,
	       radius, sd);
  
  if (target_dpi && image->resolutionX())
    {
      double scale = (double)(target_dpi) / image->resolutionX();
      if (scale < 1.0)
	box_scale (*image, scale, scale);
      else
	bilinear_scale (*image, scale, scale);
    }
 
  /* This does not look very dynamic, but it is - the real work is
     done inside the optimize2bw library - this just yields the final
     bi-level data */

  if (!threshold)
    threshold = 200;

  if (image->bps > 1)
    colorspace_gray8_to_gray1 (*image, threshold);
}

bool imageIsEmpty (Image* image, double percent, int margin)
{
  return detect_empty_page (*image, percent, margin);
}

Contours* newContours(Image* image, int low, int high,
		      int threshold,
		      int radius, double standard_deviation)
{
  optimize2bw (*image, low, high, threshold, 0, radius, standard_deviation);
  if (threshold==0)
    threshold=200;
  FGMatrix m(*image, threshold);
  return new Contours(m);
}

void deleteContours(Contours* contours)
{
  delete contours;
}

LogoRepresentation* newRepresentation(Contours* logo_contours,
			    int max_feature_no,
			    int max_avg_tolerance,
			    int reduction_shift,
			    double maximum_angle,
			    double angle_step)
{
  return new LogoRepresentation(logo_contours,
				max_feature_no,
				max_avg_tolerance,
				reduction_shift,
				maximum_angle,
				angle_step);
}

void deleteRepresentation(LogoRepresentation* representation)
{
  delete representation;
}

double matchingScore(LogoRepresentation* representation, Contours* image_contours)
{
  return representation->Score(image_contours);
}

// theese are valid after call to matchingScore()
double logoAngle(LogoRepresentation* representation)
{
  return representation->rot_angle;
}

int logoTranslationX(LogoRepresentation* representation)
{
  return representation->logo_translation.first;
}

int logoTranslationY(LogoRepresentation* representation)
{
  return representation->logo_translation.second;
}

int inverseLogoTranslationX(LogoRepresentation* representation, Image* image)
{
  return representation->CalculateInverseTranslation(image->w/2, image->h/2).first;
}

int inverseLogoTranslationY(LogoRepresentation* representation, Image* image)
{
  return representation->CalculateInverseTranslation(image->w/2, image->h/2).second;
}


void drawMatchedContours(LogoRepresentation* representation, Image* image)
{
  int tx=representation->logo_translation.first;
  int ty=representation->logo_translation.second;
  double angle=M_PI * representation->rot_angle / 180.0;

  for (unsigned int i=0; i<representation->mapping.size(); i++) {
    double trash;
    Contours::Contour transformed;
    RotCenterAndReduce(*(representation->mapping[i].first), transformed, angle, 0, 0, trash, trash);
    DrawTContour(*image, transformed, tx, ty, 0,0,255);
    DrawContour(*image, *(representation->mapping[i].second), 0,255,0);
  }
}


void imageNormalize (Image* image)
{
  normalize (*image);
}

void imageInvert (Image* image)
{
  invert (*image);
}

void imageBrightnessContrastGamma (Image* image, double brightness, double contrast, double gamma)
{
  brightness_contrast_gamma (*image, brightness, contrast, gamma);
}

void imageHueSaturationLightness (Image* image, double hue, double saturation, double lightness)
{
  hue_saturation_lightness (*image, hue, saturation, lightness);
}

// barcode recognition

#if WITHBARDECODE == 1

// barcode library
extern "C" { // missing in the library header ...
#include "barcode.h"
  
  typedef struct tagBITMAP
  {
    int bmType;
    int bmWidth;
    int bmHeight;
    int bmWidthBytes;
    unsigned char bmPlanes;
    unsigned char bmBitsPixel;
    void* bmBits;
  } BITMAP;
  
  // missing in the library header ...
  int STReadBarCodeFromBitmap (void *hBarcode, BITMAP *pBitmap, float resolution,
			       char ***bc, char ***bc_type, short photometric);
}

char** imageDecodeBarcodesExt (Image* im, const char* c,
			       unsigned int min_length,
                               unsigned int max_length, int multiple, int dirs)
{
  std::string codes = c;
  std::transform (codes.begin(), codes.end(), codes.begin(), tolower);

  std::vector<std::string> ret;

{
  const bool debug = false;
  uint16_t i;

  // a copy we can mangle
  Image* image = new Image;
  *image = *im; // deep copy
  
  int xres = 300;
  if (image->resolutionX() != 0)
    xres = image->resolutionX();
  
  // the barcode library does not support such a high bit-depth
  if (image->bps == 16)
    colorspace_16_to_8 (*image);
  
  // the library interface only handles one channel data
  if (image->spp == 3) // color crashes the library more often than not
    colorspace_rgb8_to_gray8 (*image);
  
  // the library does not appear to like 2bps ?
  if (image->bps == 2)
    colorspace_grayX_to_gray8 (*image);
  
  // now we have a 1, 4 or 8 bits per pixel GRAY image
  
  //ImageCodec::Write ("dump.tif", *image, 90, "");
  
  // The bardecode library is documented to require a 4 byte row
  // allignment. To conform this a custom allocated bitmap would
  // be required which would either require a complete Image class
  // rewrite or a extremely costly allocation and copy at this
  // location. Depending on the moon this is required or not.
  
  uint8_t* malloced_data = image->getRawData();
  {
    // required alignments
    const int base_align = 4;
    const int stride_align = 4;
    
    int stride = image->stride ();
    int new_stride = (stride + stride_align - 1) / stride_align * stride_align;
    
    // realloc the data to the maximal working set of memory we
    // might have to work with in the worst-case
    image->setRawDataWithoutDelete ((uint8_t*)
      realloc (image->getRawData(), new_stride * image->h + base_align));
    malloced_data = image->getRawData();
    uint8_t* new_data = (uint8_t*) (((long)image->getRawData() + base_align - 1) & ~(base_align-1));
    
    if (debug) {
      std::cerr << "  stride: " << stride << " aligned: " << new_stride << std::endl;
      std::cerr << "  @: " << (void*) image->getRawData()
		<< " aligned: " << (void*) new_data << std::endl;
    }
    
    if (stride != new_stride || image->getRawData() != new_data)
      {
	if (debug)
	  std::cerr << "  moving data ..." << std::endl;
	
	for (int y = image->h-1; y >= 0; --y) {
	  memmove (new_data + y*new_stride, image->getRawData() + y*stride, stride);
	  memset (new_data + y*new_stride + stride, 0xff, new_stride-stride);
       }
	
	// store new stride == width (@ 8bit gray)
	image->w = new_stride * 8 / image->bps;
	image->setRawDataWithoutDelete (new_data);
      }
  }
  
  // call into the barcode library
  void* hBarcode = STCreateBarCodeSession ();
  
  i = 0;
  STSetParameter (hBarcode, ST_READ_CODE39, &i);
  STSetParameter (hBarcode, ST_READ_CODE128, &i);
  STSetParameter (hBarcode, ST_READ_CODE25, &i);
  STSetParameter (hBarcode, ST_READ_EAN13, &i);
  STSetParameter (hBarcode, ST_READ_EAN8, &i);
  STSetParameter (hBarcode, ST_READ_UPCA, &i);
  STSetParameter (hBarcode, ST_READ_UPCE, &i);
  
  // parse the code list
  std::string c (codes);
  std::string::size_type it = 0;
  std::string::size_type it2;
  i = 1;
  do
    {
      it2 = c.find ('|', it);
      std::string code;
      if (it2 !=std::string::npos) {
	code = c.substr (it, it2-it);
	it = it2 + 1;
      }
      else
	code = c.substr (it);
      
      if (!code.empty())
	{
	  if (code == "code39")
	    STSetParameter(hBarcode, ST_READ_CODE39, &i);
	  else if (code == "code128")
	    STSetParameter(hBarcode, ST_READ_CODE128, &i);
	  else if (code == "code25")
	    STSetParameter(hBarcode, ST_READ_CODE25, &i);
	  else if (code == "ean13")
	    STSetParameter(hBarcode, ST_READ_EAN13, &i);
	  else if (code == "ean8")
	    STSetParameter(hBarcode, ST_READ_EAN8, &i);
	  else if (code == "upca")
	    STSetParameter(hBarcode, ST_READ_UPCA, &i);
	  else if (code == "upce")
	    STSetParameter(hBarcode, ST_READ_UPCE, &i);
          else if (code == "any") {
	      STSetParameter (hBarcode, ST_READ_CODE39, &i);
	      STSetParameter (hBarcode, ST_READ_CODE128, &i);
	      STSetParameter (hBarcode, ST_READ_CODE25, &i);
	      STSetParameter (hBarcode, ST_READ_EAN13, &i);
	      STSetParameter (hBarcode, ST_READ_EAN8, &i);
	      STSetParameter (hBarcode, ST_READ_UPCA, &i);
	      STSetParameter (hBarcode, ST_READ_UPCE, &i);
	  }
	  else
	    std::cerr << "Unrecognized barcode type: " << code << std::endl;
	}
    }
  while (it2 != std::string::npos);
  
  // only set if non-zero, otherwise CODE39 with chars does
  // not appear to work quite right
  if (min_length) {
    i = min_length;
    STSetParameter (hBarcode, ST_MIN_LEN, &i);
  }
  
  if (max_length) {
    i = max_length;
    STSetParameter (hBarcode, ST_MAX_LEN, &i);
  }

  // 90 degree angles differ from public API SPEC and EI built-ins
  i = (dirs & 1) | (dirs & 2) << 2 | (dirs & 4) | (dirs & 8) >> 2;
  STSetParameter(hBarcode, ST_ORIENTATION_MASK, &i);
  
  if (false) // the library has defaults?
    {
      i = 20;
      STSetParameter(hBarcode, ST_NOISEREDUCTION, &i);

      i = 1;
      STSetParameter(hBarcode, ST_DESPECKLE, &i);
      
      i = 166;
      STSetParameter(hBarcode, ST_CONTRAST, &i);
    }

  i = multiple;
  STSetParameter(hBarcode, ST_MULTIPLE_READ, &i);

  
  BITMAP bbitmap;
  bbitmap.bmType = 1; // bitmap type version, fixed v1
  bbitmap.bmWidth = image->w;
  bbitmap.bmHeight = image->h;
  bbitmap.bmWidthBytes = image->stride();
  bbitmap.bmPlanes = 1; // the library is documented to only take 1
  bbitmap.bmBitsPixel = image->bps * image->spp; // 1, 4 and 8 appeared to work
  bbitmap.bmBits = image->getRawData(); // our class' bitmap data
  
  if (debug)
    std::cerr << "  @: " << (void*) image->getRawData()
	      << ", w: " << image->w << ", h: " << image->h
	      << ", spp: " << image->spp << ", bps: " << image->bps
	      << ", stride: " << image->stride()
	      << ", res: " << xres << std::endl;
  
  char** bar_codes;
  char** bar_codes_type;
  
  // 0 == photometric min is black, but this appears to be inverted?
  int photometric = 1;
  int bar_count = STReadBarCodeFromBitmap (hBarcode, &bbitmap, xres,
					   &bar_codes, &bar_codes_type,
					   photometric);
  
  for (i = 0; i < bar_count; ++i) {
    uint32 TopLeftX, TopLeftY, BotRightX, BotRightY ;
    STGetBarCodePos (hBarcode, i, &TopLeftX, &TopLeftY,
		     &BotRightX, &BotRightY);
    //printf ("%s[%s]\n", bar_codes[i], bar_codes_type[i]);
    ret.push_back (bar_codes[i]);
    ret.push_back (bar_codes_type[i]);
  }
  
  STFreeBarCodeSession (hBarcode);
  
  // as this one needs to be free'd
  image->setRawDataWithoutDelete (malloced_data);
  delete (image); image = 0;
}
 
  char** cret = (char**)malloc (sizeof(char*) * (ret.size()+1));
  
  int i = 0;
  for (std::vector<std::string>::iterator it = ret.begin();
       it != ret.end(); ++it)
    cret[i++] = strdup (it->c_str());
  cret[i] = 0;
  
  return (char**)cret;
}

#endif

using namespace BarDecode;

namespace {

    struct comp {
        bool operator() (const scanner_result_t& a, const scanner_result_t& b) const
        {
            if (a.type < b.type) return true;
            else if (a.type > b.type) return false;
            else return (a.code < b.code);
        }
    };

    std::string filter_non_printable(const std::string& s)
    {
        std::string result;
        for (size_t i = 0; i < s.size(); ++i) {
            if ( std::isprint(s[i]) ) result.push_back(s[i]);
        }
        return result;
    }

}

char** imageDecodeBarcodes (Image* image, const char* codestr,
			    unsigned int min_length, unsigned int max_length,
                            int multiple, unsigned int line_skip, int dirs)
{
  codes_t codes = 0;
  // parse the code list
  std::string c (codestr);
  std::transform (c.begin(), c.end(), c.begin(), tolower);
  std::string::size_type it = 0;
  std::string::size_type it2;
  do
    {
      it2 = c.find ('|', it);
      std::string code;
      if (it2 !=std::string::npos) {
	code = c.substr (it, it2-it);
	it = it2 + 1;
      }
      else
	code = c.substr (it);
      
      if (!code.empty())
	{
	  if (code == "code39")
	    codes |= code39;
	  else if (code == "code128")
	    codes |= code128 | gs1_128;
	  else if (code == "code25")
	    codes |= code25i;
	  else if (code == "ean13")
	    codes |= ean13;
	  else if (code == "ean8")
	    codes |= ean8;
	  else if (code == "upca")
	    codes |= upca;
	  else if (code == "upce")
	    codes |= upce;
          else if (code == "any") {
	    codes |= ean|code128|gs1_128|code39|code25i;
	  }
	  else
	    std::cerr << "Unrecognized barcode type: " << code << std::endl;
	}
    }
  while (it2 != std::string::npos);

  const int threshold = 150;
  const directions_t directions = (directions_t)dirs;
  const int concurrent_lines = 4;

  std::map<scanner_result_t,int,comp> retcodes;
  if ( directions&(left_right|right_left) ) {
    BarDecode::BarcodeIterator<> it(image, threshold, codes, directions, concurrent_lines, line_skip);
    while (! it.end() ) {
      ++retcodes[*it];
      ++it;
    }
  }

  if ( directions&(top_down|down_top) ) {
    directions_t dir = (directions_t) ((directions&(top_down|down_top))>>1);
    BarDecode::BarcodeIterator<true> it(image, threshold, codes, dir, concurrent_lines, line_skip);
    while (! it.end() ) {
      ++retcodes[*it];
      ++it;
    }
  }
  
  std::vector<std::string> ret;
  for (std::map<scanner_result_t,int>::const_iterator it = retcodes.begin();
       it != retcodes.end();
       ++it) {
    if (it->first.type || it->second > 1)
      {
	const std::string cont = filter_non_printable(it->first.code);
	if (min_length && cont.size() < min_length)
	  continue;
	if (max_length && cont.size() > max_length)
	  continue;
	
	ret.push_back (cont);
	
	std::stringstream s; s << it->first.type;
	ret.push_back (s.str());
      }
  }

  char** cret = (char**)malloc (sizeof(char*) * (ret.size()+1));
  int i = 0;
  for (std::vector<std::string>::iterator it = ret.begin();
       it != ret.end(); ++it)
    cret[i++] = strdup (it->c_str());
  cret[i] = 0;
  
  return (char**)cret;
}
