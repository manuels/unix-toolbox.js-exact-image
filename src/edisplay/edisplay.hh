/*
 * The ExactImage library's displayy compatible command line frontend.
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

#ifndef EDISPLAY_HH
#define EDISPLAY_HH

#include "Image.hh"

// display stuff
#include "X11Helper.hh"
#include "EvasHelper.hh"

#include "Timer.hh"

#include <string>
#include <vector>

class Viewer {
public:
  
  Viewer(const std::vector<std::string>& _images)
    : images(_images), evas_data(0), zoom(100), evas_image(0) {
    it = images.begin();
    image = new Image;
  }
  
  virtual ~Viewer() {
    if (evas_data) {
      delete (evas_data); evas_data = 0;
    }
    delete (image); image = 0;
  }
  
  bool Load ();
  bool Next ();
  bool Previous ();

  int Run (bool opengl = false);
  
protected:
  
  void ImageToEvas ();
  EvasImage* ImageToEvas (Image*, EvasImage* = 0);
  
  void Zoom (double factor);
  void Move (int _x, int _y);
  
  int Window2ImageX (int x);
  int Window2ImageY (int y);
  
  void UpdateOSD (const std::string& str1, const std::string& str2);
  void AlphaOSD (int a);
  void TickOSD ();

  void SetOSDZoom ();
  
  virtual void ImageLoaded () {};
  virtual void ImageClicked (unsigned int x, unsigned int y, int button) {};
  virtual bool ImageKey(KeySym keysym) { return false; };
  
private:
  const std::vector<std::string>& images;
  std::vector<std::string>::const_iterator it;
  
  // Image
protected:
  Image* image;
private:
  uint8_t* evas_data;
  
  // on screen display
  EvasRectangle* evas_osd_rect;
  EvasText* evas_osd_text1;
  EvasText* evas_osd_text2;
  Utility::Timer osd_timer;
  
  int zoom;
  int channel;
  
  // X11 stuff
  Display* dpy;
  int scr;
  Visual* visual;
  Window  win;
  int depth;
  
  // evas
  EvasCanvas* evas;
  EvasImage* evas_image;
  EvasImage* evas_bgr_image;
  
protected:
  // real canvas payload
  std::vector<EvasImage*> evas_content;
};

#endif // EDISPLAY_HH
