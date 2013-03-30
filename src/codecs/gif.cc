/*
 * Copyright (C) 2006 - 2009 René Rebe
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

#include <gif_lib.h>

#include "gif.hh"
#include "Colorspace.hh"

#include <iostream>

/* The way Interlaced image should. */
static const int InterlacedOffset[] = { 0, 4, 2, 1 };

/* be read - offsets and jumps... */
static const int InterlacedJumps[] = { 8, 8, 4, 2 };

static int GIFInputFunc (GifFileType* t, GifByteType* mem, int len)
{
  std::istream* stream = (std::istream*)t->UserData;
  size_t nbytes = stream->tellg();
  stream->read((char*)mem, len);
  nbytes = (size_t)stream->tellg() - nbytes;
  return nbytes;
}

static int GIFOutputFunc (GifFileType* t, const GifByteType* mem, int len)
{
  std::ostream* stream = (std::ostream*)t->UserData;
  stream->write ((char*)mem, len);
  return len;
}

int GIFCodec::readImage (std::istream* stream, Image& image, const std::string& decompres)
{
  { // quick magic check
    char buf [3];
    stream->read (buf, sizeof (buf));
    stream->seekg (0);
    if (buf[0] != 'G' || buf[1] != 'I' || buf[2] != 'F')
      return false;
  }
  
  GifFileType* GifFile;
  GifRecordType RecordType;
  GifByteType* Extension;
  ColorMapObject *ColorMap = NULL;
  int ExtCode;
  
  if ((GifFile = DGifOpen (stream, &GIFInputFunc)) == NULL)
    {
      PrintGifError();
      return false;
    }
  
  image.spp = 1;
  image.bps = 8;
  image.setResolution(0, 0);
  image.resize (GifFile->SWidth,GifFile->SHeight);
  
  /* Scan the content of the GIF file and load the image(s) in: */
  do {
    if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
      PrintGifError();
      return false;
    }
    
    int Row, Col, Width, Height;
    
    switch (RecordType) {
    case IMAGE_DESC_RECORD_TYPE:
      if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
	PrintGifError();
	return false;
      }
      
      Row = GifFile->Image.Top; /* Image Position relative to Screen. */
      Col = GifFile->Image.Left;
      Width = GifFile->Image.Width;
      Height = GifFile->Image.Height;
      
      if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
	  GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
	std::cerr << "Image not in screen dimension, aborted." << std::endl;
	return false;
      }
      if (GifFile->Image.Interlace) {
	/* Need to perform 4 passes on the images: */
	for (int i = 0; i < 4; ++i)
	  for (int j = Row + InterlacedOffset[i]; j < Row + Height;
	       j += InterlacedJumps[i]) {
	    if (DGifGetLine(GifFile, &image.getRawData()[j*image.stride()+Col],
			    Width) == GIF_ERROR) {
	      PrintGifError();
	      return false;
	    }
	  }
      }
      else {
	for (int i = 0; i < Height; ++i) {
	  if (DGifGetLine(GifFile, &image.getRawData()[Row++ * image.stride()+Col],
			  Width) == GIF_ERROR) {
	    PrintGifError();
	    return false;
	  }
	}
      }
      break;
    case EXTENSION_RECORD_TYPE:
      /* Skip any extension blocks in file: */
      if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
	PrintGifError();
	return false;
      }
      while (Extension != NULL) {
	if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
	  PrintGifError();
	  return false;
	}
      }
      break;
    case TERMINATE_RECORD_TYPE:
      break;
    default:		    /* Should be traps by DGifGetRecordType. */
      break;
    }
  }
  while (RecordType != TERMINATE_RECORD_TYPE);
  
  ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap :
	      GifFile->SColorMap);
  
  uint16_t rmap [ColorMap->ColorCount];
  uint16_t gmap [ColorMap->ColorCount];
  uint16_t bmap [ColorMap->ColorCount];
  for (int i = 0; i < ColorMap->ColorCount; ++i) {
    rmap[i] = ColorMap->Colors[i].Red << 8;
    gmap[i] = ColorMap->Colors[i].Green << 8;
    bmap[i] = ColorMap->Colors[i].Blue << 8;
  }
  
  // convert colormap to our 16bit "TIFF"format
  colorspace_de_palette (image, ColorMap->ColorCount, rmap, gmap, bmap);
  
  EGifCloseFile(GifFile);

  return true;
}

bool GIFCodec::writeImage (std::ostream* stream, Image& image, int quality,
			   const std::string& compress)
{
  GifFileType* GifFile;
  GifByteType* Ptr;
  
  if ((GifFile = EGifOpen (stream, &GIFOutputFunc)) == NULL)
    {
      std::cerr << "Error preparing GIF file for writing." << std::endl;
      return false;
    }
  
  int ColorMapSize = 256;
  
  // later use our own colormap generation
  ColorMapObject* OutputColorMap = MakeMapObject(ColorMapSize, NULL);
  if (!OutputColorMap)
    return false;
  
  GifByteType* OutputBuffer =
    (GifByteType*) malloc(image.w * image.h *  sizeof(GifByteType));
  if (!OutputBuffer)
    return false;
  
  GifByteType
    *RedBuffer = new GifByteType [image.w*image.h],
    *GreenBuffer = new GifByteType [image.w*image.h],
    *BlueBuffer = new GifByteType [image.w*image.h];
  GifByteType
    *rptr = RedBuffer,
    *gptr = GreenBuffer,
    *bptr = BlueBuffer;
 
  for (Image::iterator it = image.begin(); it != image.end(); ++it) {
    uint16_t r = 0, g = 0, b = 0;
    *it;
    it.getRGB (&r, &g, &b);
    *rptr++ = r;
    *gptr++ = g;
    *bptr++ = b;
  }
   
  
  if (QuantizeBuffer(image.w, image.h, &ColorMapSize,
		     RedBuffer, GreenBuffer, BlueBuffer,
		     OutputBuffer, OutputColorMap->Colors) == GIF_ERROR) {
    return false;
  }
  
  std::cerr << "Writing uncompressed GIF file with "
	    << (int)ColorMapSize << " colors." << std::endl;
  
  if (EGifPutScreenDesc(GifFile, image.w, image.h,
			ColorMapSize, 0, OutputColorMap) == GIF_ERROR ||
      EGifPutImageDesc(GifFile, 0, 0, image.w, image.h,
		       FALSE, NULL) == GIF_ERROR)
    {
      std::cerr << "Error writing GIF header." << std::endl;
      return false;
    }
  Ptr = OutputBuffer;
  for (int i = 0; i < image.h; ++i) {
    if (EGifPutLine(GifFile, Ptr, image.w) == GIF_ERROR)
      {
	std::cerr << "Error writing GIF header." << std::endl;
	return false;
      }
    
    Ptr += image.w;
  }
  free (OutputBuffer);

  delete (RedBuffer); delete (GreenBuffer); delete (BlueBuffer);

  EGifCloseFile(GifFile);
  return true;
}

GIFCodec gif_loader;
