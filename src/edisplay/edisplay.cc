/*
 * The ExactImage library's displayy compatible command line frontend.
 * Copyright (C) 2006 - 2012 René Rebe
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

#include "config.h"

#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Evas.h"
#include "Evas_Engine_Software_X11.h"
#if EVASGL == 1
#include "Evas_Engine_GL_X11.h"
#endif

#include <endian.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "ArgumentList.hh"
using namespace Utility;

// imaging stuff

#include "Codecs.hh"
#include "Colorspace.hh"

#include "rotate.hh"

#include "edisplay.hh"

#include "Endianess.hh"

using std::cout;
using std::cerr;
using std::endl;

static uint32_t evas_bgr_image_data[] = {
  0x999999, 0x666666,
  0x666666, 0x999999};

int Viewer::Window2ImageX (int x)
{
  return (x - evas_image->X() ) * 100 / zoom;
}

int Viewer::Window2ImageY (int y)
{
  return (y - evas_image->Y() ) * 100 / zoom;
}

void Viewer::Zoom (double f)
{
  // to keep the view centered
  int xcent = Window2ImageX (evas->OutputWidth()/2);
  int ycent = Window2ImageY (evas->OutputHeight()/2);
  
  int z = zoom;
  zoom = (int) (f * zoom);
  
  if (f > 1.0 && zoom == z)
    ++zoom;
  else if (zoom <= 0)
    zoom = 1;
  
  SetOSDZoom ();
  
  Evas_Coord w = (Evas_Coord) (zoom * image->w / 100);
  Evas_Coord h = (Evas_Coord) (zoom * image->h / 100);
  
  evas_bgr_image->Resize (w, h);
  
  // recenter view
  for (std::vector<EvasImage*>::iterator it = evas_content.begin();
       it != evas_content.end(); ++it) {
    (*it)->Resize (w, h);
    (*it)->ImageFill (0, 0, w, h);
    (*it)->Move (- (xcent * zoom / 100 - (evas->OutputWidth() / 2)),
		 - (ycent * zoom / 100 - (evas->OutputHeight() / 2)) );
  }

  // limit / clip accordingly
  Move (0, 0);

  if (true) {
    // resize X window accordingly
    X11Window::Resize (dpy, win,
		       std::min(w, X11Window::Width(dpy, 0)),
		       std::min(h, X11Window::Height(dpy, 0)));
  }
}

void Viewer::Move (int _x, int _y)
{
  Evas_Coord x = evas_image->X() + _x;
  Evas_Coord y = evas_image->Y() + _y;
  
  Evas_Coord w = evas_image->Width();
  Evas_Coord h = evas_image->Height();
  
  // limit
  if (x + w < evas->OutputWidth() )
      x = evas->OutputWidth() - w;
  if (x > 0)
    x = 0;
  
  if (y + h < evas->OutputHeight() )
    y = evas->OutputHeight() - h;
  if (y > 0)
    y = 0;
  
  for (std::vector<EvasImage*>::iterator it = evas_content.begin();
       it != evas_content.end(); ++it)
    (*it)->Move (x, y);
}


void Viewer::UpdateOSD (const std::string& str1, const std::string& str2)
{
  evas_osd_text1->Text (str1);
  evas_osd_text2->Text (str2);
  evas_osd_rect->Resize (std::max(evas_osd_text1->Width(), evas_osd_text2->Width()) + 4,
			 evas_osd_text1->Height() + evas_osd_text2->Height() + 8);
  AlphaOSD (0xFF);
  osd_timer.Reset ();
}

void Viewer::AlphaOSD (int a)
{
  evas_osd_text1->Color (0xFF, 0xFF, 0xFF, a);
  evas_osd_text2->Color (0xFF, 0xFF, 0xFF, a);
  evas_osd_rect->Color (0x40, 0x40, 0x40, std::max (a-92, 0));
}

void Viewer::TickOSD ()
{
  const int d = osd_timer.Delta ();
  const int ps = osd_timer.PerSecond ();
  if (d > ps && d <= 2*ps) {
    int a = std::max (0xFF - (d - ps) * 0xff / ps, 0);
    AlphaOSD (a);
  }
}

void Viewer::SetOSDZoom ()
{
  std::stringstream s1, s2;
  s1 << "Zoom: " << zoom << "%";
  s2 << "Anti-alias: " << (evas_image->SmoothScale() ? "yes" : "no");
  
  UpdateOSD (s1.str(), s2.str());
}

int Viewer::Run (bool opengl)
{
  // TODO: move to the X11Helper ...
  
  XSetWindowAttributes attr;
  XClassHint           chint;
#if 0
  XSizeHints           szhints;
#endif

  dpy = XOpenDisplay (NULL);
  if (!dpy) {
    return false;
  }

  scr = DefaultScreen (dpy);

  Window root = RootWindow(dpy, scr);

  attr.backing_store = NotUseful;
  attr.border_pixel = 0;
  attr.background_pixmap = None;
  attr.event_mask =
    ExposureMask |
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
    KeyPressMask | KeyReleaseMask |
    StructureNotifyMask;
  attr.bit_gravity = ForgetGravity;
  /*  attr.override_redirect = 1; */

  visual = DefaultVisual (dpy, scr);
  depth = DefaultDepth(dpy, scr);
  attr.colormap = DefaultColormap(dpy, scr);
  
  int win_w = 1;
  int win_h = 1;
  
  win = XCreateWindow (dpy, root,
		       (X11Window::Width(dpy, 0) - win_w) / 2,
		       (X11Window::Height(dpy, 0) - win_h) / 2,
		       win_w, win_h,
		       0,
		       depth,
		       InputOutput,
		       visual,
		       /*CWOverrideRedirect | */ 
		       CWBackingStore | CWColormap |
		       CWBackPixmap | CWBorderPixel |
		       CWBitGravity | CWEventMask, &attr);

  XStoreName (dpy, win, "edisplay");
  chint.res_name = "edisplay";
  chint.res_class = "Main";
  XSetClassHint (dpy, win, &chint);
  
#if 0
  szhints.flags = PMinSize | PMaxSize | PSize | USSize;
  szhints.min_width = szhints.max_width = win_w;
  szhints.min_height = szhints.max_height = win_h;
  XSetWMNormalHints(dpy, win, &szhints);
#endif

  XSync(dpy, False);
  
  evas = new EvasCanvas ();
  if (!opengl)
    {
      evas->OutputMethod ("software_x11");
      evas->OutputSize (win_w, win_h);
      evas->OutputViewport (0, 0, win_w, win_h);
      
      Evas_Engine_Info_Software_X11* einfo;
      
      einfo = (Evas_Engine_Info_Software_X11*) evas->EngineInfo ();
      
      /* the following is specific to the engine */
#ifdef WITHEVAS_X11_CONNECTION
      einfo->info.connection = dpy;
#else
      einfo->info.display = dpy;
#endif
      einfo->info.visual = visual;
      einfo->info.colormap = attr.colormap;
      einfo->info.drawable = win;
      einfo->info.depth = depth;
      einfo->info.rotation = 0;
      einfo->info.debug = 0;
      
      evas->EngineInfo ( (Evas_Engine_Info*)einfo );
    }
#if EVASGL == 1
  else
    {
      evas->OutputMethod ("gl_x11");
      evas->OutputSize (win_w, win_h);
      evas->OutputViewport (0, 0, win_w, win_h);
      
      Evas_Engine_Info_GL_X11* einfo;
      
      einfo = (Evas_Engine_Info_GL_X11*) evas->EngineInfo ();
      
      /* the following is specific to the engine */
      einfo->info.display = dpy;
#ifdef WITHEVAS_X11_SCREEN
      einfo->info.screen = DefaultScreen(dpy);
      einfo->info.visual = einfo->func.best_visual_get(einfo);
      einfo->info.colormap = einfo->func.best_colormap_get(einfo);
#else
      einfo->info.visual = einfo->func.best_visual_get(dpy, DefaultScreen(dpy));
      einfo->info.colormap = einfo->func.best_colormap_get(dpy, DefaultScreen(dpy));
#endif
      einfo->info.drawable = win;
      einfo->info.depth = depth;
      
      evas->EngineInfo ( (Evas_Engine_Info*)einfo );
    }
#endif

  /* Setup */
  evas->FontPathPrepend ("/usr/X11/lib/X11/fonts/TTF/");
  evas->FontPathPrepend ("/usr/X11/lib/X11/fonts/TrueType/");
  evas->FontPathPrepend ("/opt/e17/share/evas/data/");
  evas->FontPathPrepend ("/usr/X11/share/fonts/TTF/");
 
  if (true) {
    evas->ImageCache (1024 * 1024);
    evas->FontCache (256 * 1024);
  }
  else {
    evas->ImageCache (0);
    evas->FontCache (0);
  }
  
  evas_bgr_image = new EvasImage (*evas);
  evas_bgr_image->SmoothScale (false);
  evas_bgr_image->Layer (0);
  evas_bgr_image->Move (0,0);

  evas_bgr_image->ImageSize (2,2);
  evas_bgr_image->ImageFill (0,0,24,24);
  
  evas_bgr_image->Resize (win_w,win_h);
  
  evas_bgr_image->DataUpdateAdd (0,0,2,2);
  evas_bgr_image->SetData ((uint8_t*)evas_bgr_image_data);
  evas_bgr_image->Show ();
  
  // OSD
  
  evas_osd_text1 = new EvasText (*evas);
  evas_osd_text1->Layer (10);
  evas_osd_text1->Move (4,4);
  evas_osd_text1->Font ("Vera", 10);
  evas_osd_text1->Show ();

  evas_osd_text2 = new EvasText (*evas);
  evas_osd_text2->Layer (10);
  evas_osd_text2->Move (4,8 + evas_osd_text1->Height());
  evas_osd_text2->Font ("Vera", 10);
  evas_osd_text2->Show ();

  evas_osd_rect = new EvasRectangle (*evas);
  evas_osd_rect->Layer (5);
  evas_osd_rect->Move (2,2);
  evas_osd_rect->Show ();
  
  AlphaOSD (0);
  
  bool image_loaded;
  image_loaded = Load ();
  if (!image_loaded)
    image_loaded = Next ();
  
  XMapWindow (dpy, win);

  bool quit = false;
  Utility::Timer timer;
  int dnd_x = 0, dnd_y = 0;
  bool dnd_moved = false;

  while (image_loaded && !quit)
    {
      // process X11 events ...
      // TODO: move into X11 Helper ...
      
      XEvent ev;
      while (XCheckMaskEvent (dpy,
			      ExposureMask |
			      StructureNotifyMask |
			      KeyPressMask |
			      KeyReleaseMask |
			      ButtonPressMask |
			      ButtonReleaseMask |
			      PointerMotionMask, &ev))
	{
	  Evas_Button_Flags flags = EVAS_BUTTON_NONE;
	  switch (ev.type)
	    {
	    case ButtonPress:
	      {
		//evas->EventFeedMouseMove (ev.xbutton.x, ev.xbutton.y);
		//evas->EventFeedMouseDown (ev.xbutton.button, flags);
		dnd_moved = false;
		switch (ev.xbutton.button) {
		case 1:
		  dnd_x = ev.xbutton.x;
		  dnd_y = ev.xbutton.y;
		  break;
		case 4:
		  if (ev.xbutton.state & ControlMask)
		    Zoom (1.1);
		  else
		    Move (0, 40);
		  break;
		case 5:
		  if (ev.xbutton.state & ControlMask)
		    Zoom (1.0/1.1);
		  else
		    Move (0, -40);
		  break;
		case 6:
		  Move (40, 0);
		  break;
		case 7:
		  Move (-40, 0);
		  break;
		}
	      }
	      break;
	    case ButtonRelease:
	      if (!dnd_moved)
		ImageClicked(Window2ImageX (ev.xmotion.x),
			     Window2ImageY (ev.xmotion.y),
			     ev.xbutton.button);
	      break;
	      
	    case MotionNotify:
	      // evas->EventFeedMouseMove (ev.xmotion.x, ev.xmotion.y);
	      
	      // dragged around:
	      dnd_moved = true;
	      if (ev.xmotion.state & Button1Mask)
		{
		  int dx = ev.xmotion.x - dnd_x;
		  int dy = ev.xmotion.y - dnd_y;
		  dnd_x = ev.xmotion.x;
		  dnd_y = ev.xmotion.y;
		  Move (dx, dy);
		}
	      else // no handled button, update OSD
		{
		  std::stringstream s1, s2;
		  int x = Window2ImageX (ev.xmotion.x);
		  int y = Window2ImageY (ev.xmotion.y);
		  
		  if (x <= image->w && y <= image->h) {
		    uint16_t r = 0, g = 0, b = 0;
		    
		    Image::iterator it = image->begin();
		    it = it.at (x, y);
		    (*it).getRGB (&r, &g, &b);
		    
		    s1 << "x: " << x << ", y: " << y;
		    s2 << "r: " << std::hex << r << ", g: " << g << ", b: " << b;
		  }
		  else {
		    s1 << "x: , y:";
		    s2 << "r: , g: , b:";
		  }
		  
		  UpdateOSD (s1.str(), s2.str());
		}
	      break;
	    
	    case KeyPress:
	      KeySym ks;	      
	      XLookupString ((XKeyEvent*)&ev, 0, 0, &ks, NULL);
	      if (ImageKey(ks))
		break;
	      
	      //cout << "key: " << ev.xkey.keycode << endl;
	      //cout << "sym: " << ks << endl;
	      switch (ks)
		{
		case XK_1:
		  zoom = 100;
		  Zoom (1);
		  break;

		case XK_plus:
		  Zoom (2);
		  break;
		  
		case XK_minus:
		  Zoom (.5);
		  break;
		  
		case XK_Left:
		  Move (40, 0);
		  break;
		
		case XK_Right:
		  Move (-40, 0);
		  break;
		  
		case XK_Up:
		  Move (0, 40);
		  break;
		  
		case XK_Down:
		  Move (0, -40);
		  break;
		  
		case XK_Page_Up:
		  Move (0, 160);
		  break;
		  
		case XK_Page_Down:
		  Move (0, -160);
		  break;

		case XK_space:
		  image_loaded = Next ();
		  break;

		case XK_BackSpace:
		  image_loaded = Previous ();
		  break;
		  
		case XK_ISO_Left_Tab:
		  channel -= 2; // +1 below ...
		case XK_Tab:
		  ++channel;
		  if (channel > 6)
		    channel = 0;
		  else if (channel < 0)
		    channel = 6;
		  ImageToEvas ();
		  {
		    std::string s1, s2;
		    s1 = "Color channel";
		    switch (channel) {
		    case 1: s2 = "just R"; break;
		    case 2: s2 = "just G"; break;
		    case 3: s2 = "just B"; break;
		    case 4: s2 = "R intensity"; break;
		    case 5: s2 = "G intensity"; break;
		    case 6: s2 = "B intensity"; break;
		    default: s2 = "NONE";
		    }
		    s2 = "dropout: " + s2;
		    UpdateOSD (s1, s2);
		  }
		  break;
		  
		case XK_a:
		  evas_image->SmoothScale (!evas_image->SmoothScale());
		  // schedule the update
		  evas->DamageRectangleAdd (0, 0,
					    evas->OutputWidth(),
					    evas->OutputHeight());
		  
		  SetOSDZoom ();
		  break;

		case XK_i:
		  invert (*image);
		  ImageToEvas ();
		  AlphaOSD (0);
		  break;

		case XK_greater:
		  rotate (*image, 90, image->begin());
		  ImageToEvas ();
		  AlphaOSD (0);
		  break;
		
		case XK_less:
		  rotate (*image, -90, image->begin());
		  ImageToEvas ();
		  AlphaOSD (0);
		  break;
		  
		case XK_q:
		  quit = true;
		  break;
		}
	      
	      break;
	      
	    case Expose:
	      evas->DamageRectangleAdd (ev.xexpose.x,
					ev.xexpose.y,
					ev.xexpose.width,
					ev.xexpose.height);
	      break;
	      
	    case ConfigureNotify:
	      evas_bgr_image->Resize (ev.xconfigure.width,
				      ev.xconfigure.height);
	      evas->OutputSize (ev.xconfigure.width,
				ev.xconfigure.height);
	      evas->OutputViewport (0, 0,
				    ev.xconfigure.width,
				    ev.xconfigure.height);
	      // limit/clip
	      Move (0, 0);
	      break;
	      
	    default:
	      break;
	    }
	}
      TickOSD ();
      evas->Render ();
      XFlush (dpy);
      usleep (10000);
    }
  
  if (evas_image) {
    delete evas_image;
    evas_image = 0;
  }
  
  delete evas_osd_text1; evas_osd_text1 = 0;
  delete evas_osd_text2; evas_osd_text2 = 0;
  delete evas_osd_rect;  evas_osd_rect = 0;
  delete evas_bgr_image; evas_bgr_image = 0;
  
  delete evas;
  evas = 0;
  
  return 0;
}

bool Viewer::Next ()
{
  std::vector<std::string>::const_iterator ref_it = it;
  do {
    ++it;
    if (it == images.end())
      it = images.begin();
    
    if (Load ())
      return true;
  }
  while (it != ref_it);
  return false;
}

bool Viewer::Previous ()
{
  std::vector<std::string>::const_iterator ref_it = it;
  do {
    if (it == images.begin())
      it = images.end();
    --it;
    if (Load ())
      return true;
  }
  while (it != ref_it);
  return false;
}

bool Viewer::Load ()
{
  image->setRawData(0);
  
  // reset channel filter
  channel = 0;
  
  if (!ImageCodec::Read(*it, *image)) {
    // TODO: fix to gracefully handle this
    cerr << "Error loading file: " << *it << endl;
    return false;
  }
  
  // notify early to allow sub-class to make modifications
  ImageLoaded ();
  
  if (false)
  cerr << "Loaded: '" << *it
       << "', " << image->w << "x" << image->h
       << " @ " << image->resolutionX() << "x" << image->resolutionY()
       << " dpi - spp: " << image->spp << ", bps: " << image->bps << endl;
  
  // convert colorspace
  if (image->bps == 16)
    colorspace_16_to_8 (*image);
  
  // convert any gray to RGB
  if (image->spp == 1)
    colorspace_grayX_to_rgb8 (*image);
  
  if (image->bps != 8 || (image->spp != 3 && image->spp != 4)) {
    cerr << "Unsupported colorspace. bps: " << image->bps
	 << ", spp: " << image->spp << endl;
    cerr << "If possible please send a test image to rene@exactcode.de."
	 << endl;
    return false;
  }

  if (!image->getRawData()) {
    cerr << "image data not loaded?"<< endl;
    return false;
  }
  
  ImageToEvas ();
  
  std::string title = *it;
  title.insert (0, "edisplay: ");
  XStoreName (dpy, win, title.c_str());
  
  return true;
}

void Viewer::ImageToEvas ()
{
  if (!evas_image) {
    evas_image = new EvasImage (*evas);
    evas_image->SmoothScale (false);
    evas_image->Layer (1);
    evas_image->Move (0, 0);
    evas_image->Resize (image->w,image->h);
    evas_content.push_back(evas_image);
  } else {
    evas_image->SetData (0);
  }

  evas_image = ImageToEvas (image, evas_image);
  
  // position and resize, keep zoom
  if (false) {
    zoom = 100;
  }
  Zoom (1.0);
}

EvasImage* Viewer::ImageToEvas (Image* image, EvasImage* eimage)
{
  uint8_t* evas_data = 0;
  
  if (!eimage) {
    eimage = new EvasImage (*evas);
    eimage->SmoothScale (false);
    eimage->Layer (1);
    eimage->Move (0, 0);
    eimage->Resize (image->w,image->h);
  } else {
    eimage->SetData (0);
  }
  
  evas_data = (uint8_t*) realloc (evas_data, image->w*image->h*4);
  uint8_t* src_ptr = image->getRawData();
  uint8_t* dest_ptr = evas_data;

  const int spp = image->spp; 
  
  if (channel == 0)
    {
      for (int y=0; y < image->h; ++y)
        for (int x=0; x < image->w; ++x, dest_ptr +=4, src_ptr += spp) {
          if (!Exact::NativeEndianTraits::IsBigendian) {
            dest_ptr[0] = src_ptr[2];
            dest_ptr[1] = src_ptr[1];
            dest_ptr[2] = src_ptr[0];
            if (spp == 4)
              dest_ptr[3] = src_ptr[3]; // alpha
          }
          else {
            dest_ptr[1] = src_ptr[0];
            dest_ptr[2] = src_ptr[1];
            dest_ptr[3] = src_ptr[2];
            if (spp == 4)
              dest_ptr[0] = src_ptr[3]; // alpha
          }
        }
    }
  else
    {
      bool intensity = channel > 3;
      int ch = (channel-1) % 3;
      
      for (int y=0; y < image->h; ++y)
        for (int x=0; x < image->w; ++x, dest_ptr +=4, src_ptr += spp) {
          if (!Exact::NativeEndianTraits::IsBigendian) {
            dest_ptr[0] = dest_ptr[1] = dest_ptr[2] =
              intensity ? src_ptr[ch] : 0;
            if (!intensity)
	      dest_ptr[2-ch] = src_ptr[ch];
          }
          else {
            dest_ptr[1] = dest_ptr[2] = dest_ptr[3] =
              intensity ? src_ptr[ch] : 0;
            if (!intensity)
              dest_ptr[1+ch] = src_ptr[ch];
          }
        }
    }
  
  eimage->Alpha (spp == 4);
  eimage->Resize (image->w, image->h);
  eimage->ImageSize (image->w, image->h);
  eimage->ImageFill (0, 0, image->w,image->h);
  eimage->SetData (evas_data);
  eimage->DataUpdateAdd (0, 0, image->w,image->h);
  eimage->Show ();
  
  return eimage;
}

Viewer* __attribute__ ((weak)) createViewer(const std::vector<std::string>& args)
{
  return new Viewer(args);
}

int main (int argc, char** argv)
{
  ArgumentList arglist (true); // enable residual gathering
  
  // setup the argument list
  Argument<bool> arg_help ("", "help",
                           "display this help text and exit");
  arglist.Add (&arg_help);
#if EVASGL == 1
  Argument<bool> arg_gl ("", "gl",
			 "Utilize OpenGL for rendering");
  arglist.Add (&arg_gl);
#endif
  
  // parse the specified argument list - and maybe output the Usage
  if (!arglist.Read (argc, argv) || arg_help.Get() == true || arglist.Residuals().empty())
    {
      cerr << "Exact image viewer (edisplay)."
	   << endl << "Version " VERSION
	   <<  " - Copyright (C) 2006 - 2010 by René Rebe" << std::endl
	   << "Usage:" << endl;
      
      arglist.Usage (cerr);
      return 1;
    }
  
  Viewer* viewer= createViewer(arglist.Residuals());
  
  int ret = viewer->Run (
#if EVASGL == 1
			 arg_gl.Get()
#else
			 false
#endif
			 );
  delete viewer; viewer = 0;
  return ret;
}
