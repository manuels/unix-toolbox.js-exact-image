#include "ContourUtility.hh"
#include <cmath>
#include <cstdlib>
//#include <iostream>
#include <assert.h>
#include <stdio.h>

void CenterAndReduce(const Contours::Contour& source,
		     Contours::Contour& dest,
		     unsigned int shift, // integer coordinate bit reduction
		     double& drx, // returned centroid
		     double& dry  // returned centroid
		     )
{
  unsigned int rx=0;
  unsigned int ry=0;
  unsigned int lastx=(unsigned int)-1;
  unsigned int lasty=(unsigned int)-1;
  for (unsigned int i=0; i<source.size(); i++) {
    unsigned int x=(int)source[i].first >> shift;
    unsigned int y=(int)source[i].second >> shift;
    if (x != lastx || y != lasty) {
      dest.push_back(std::pair<unsigned int, unsigned int>(x, y));
      lastx=x;
      lasty=y;
      rx+=x;
      ry+=y;
    }
  }
  drx=((double) rx / (double) dest.size());
  dry=((double) ry / (double) dest.size());
}


void RotCenterAndReduce(const Contours::Contour& source,
			Contours::Contour& dest,
			double phi, // clockwise rotation in radians
			unsigned int add, // added to all coordinates to avoid negative values,
                                          // should be to set to image.w+image.h
			unsigned int shift, // integer coordinate bit reduction
			double& drx, // returned centroid 
			double& dry  // returned centroid
			)
{
  Contours::Contour tmp;

  double c=cos(phi);
  double s=sin(phi);

  int lastx=0;
  int lasty=0;
  for (unsigned int i=0; i<source.size(); i++) {
    double dx=(double)source[i].first;
    double dy=(double)source[i].second;

    double nx=c*dx - s*dy;
    double ny=s*dx + c*dy;

    int x=(int)nx + (int)add;
    int y=(int)ny + (int)add;

    // in case of gab, place an intermediate contour pixel
    if (i > 0 && (abs(x-lastx) > 1 || abs(y-lasty) > 1)) {
      tmp.push_back(std::pair<unsigned int, unsigned int>((x+lastx)/2, (y+lasty)/2));
    }

    tmp.push_back(std::pair<unsigned int, unsigned int>(x,y));
    lastx=x;
    lasty=y;
  }

  CenterAndReduce(tmp, dest, shift, drx, dry);
}

double L1Dist(const Contours::Contour& a,
	      const Contours::Contour& b,
	      double drax, // a centroid
	      double dray, // a centroid
	      double drbx, // b centroid
	      double drby, // b centroid
	      unsigned int shift,   // reduction bits (in case a and b are reduced) or 0
	      double& transx, // returned translation for a
	      double& transy  // returned translation for a
	      )
{
  double factor=(double)(1 << shift);
  transx= (drbx-drax)*factor;
  transy= (drby-dray)*factor;

  int dx= (int)(drbx-drax);
  int dy= (int)(drby-dray);
  double sum=.0;

  int best=1000000;
  int opt=0;
  int delta=0;
  int lastpos=0;
  //bool forward=true;
  for (unsigned int i=0; i<a.size(); i++) {
    if (i>0) {
      delta=abs((int)a[i].first-(int)a[i-1].first)+abs((int)a[i].second-(int)a[i-1].second);
      opt=best-delta;
      best+=delta;
    }

    int pos=lastpos;
    for (unsigned int j=0; j<b.size(); j++) {
      int current=abs(dx+(int)a[i].first-(int)b[pos].first)+abs(dy+(int)a[i].second-(int)b[pos].second);

      /* sanity check
      if (current < 0) {
	std::cout << "? " << current << "\t"
		  << abs(dx+(int)a[i].first-(int)b[pos].first) << "\t"
		  << abs(dy+(int)a[i].second-(int)b[pos].second) << std::endl;
	std::cout << dx << "\t" << (int)a[i].first << "\t" << (int)b[pos].first << std::endl;
	std::cout << dy << "\t" << (int)a[i].second << "\t" << (int)b[pos].second << std::endl;	
      } */

      if (current < best) {
	best=current;
	//forward=(pos>lastpos);
	lastpos=pos;
	if (best == opt)
	  j=b.size();
      }
      else if (current > best) {
	int skip=((current - best) - 1) / 2;
	j+=skip;
	pos+=skip; //(forward) ? skip : -skip;
      }

      //if (forward) {
	pos++;
	if (pos >= (int)b.size())
	  pos-=b.size();
	//} else {
	//	pos--;
	//if (pos < 0)
	//  pos+=b.size();
	//}
    }

    sum += best;
  }
  
  return sum*factor;
}

// not very efficient, yet effective
static void PutPixel(Image& img, int x, int y, uint16_t R, uint16_t G,  uint16_t B)
{
  Image::iterator p=img.begin();
  p=p.at(x,y);
  p.setRGB(R, G, B);
  p.set(p);
}


void DrawContour(Image& img, const Contours::Contour& c, unsigned int r, unsigned int g, unsigned int b)
{
  for (unsigned int i=0; i<c.size(); i++) {
    PutPixel(img, c[i].first, c[i].second, r, g, b);
  }
}


void DrawTContour(Image& img, const Contours::Contour& c, unsigned int tx, unsigned int ty, unsigned int r, unsigned int g, unsigned int b)
{
  for (unsigned int i=0; i<c.size(); i++) {
    int x=c[i].first+tx;
    int y=c[i].second+ty;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x, y, r, g, b);
  }
}

/*
void DrawContour(Image& img, const Contours::Contour& c, unsigned int r, unsigned int g, unsigned int b)
{
  int xsum=0;
  int ysum=0;
  for (unsigned int i=0; i<c.size(); i++) {
    PutPixel(img, c[i].first, c[i].second, r, g, b);
    xsum+=c[i].first;
    ysum+=c[i].second;
  }

  xsum /= c.size();
  ysum /= c.size();

  for (int dx=-10; dx<=10; dx++) {
    int x=xsum+dx;
    int y=ysum;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x,y, r, g, b);
  }
  for (int dy=-10; dy<=10; dy++) {
    int x=xsum;
    int y=ysum+dy;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img,x,y, r, g, b);
  }
 
  Contours::Contour trash;
  double tx=0;
  double ty=0;
  CenterAndReduce(c,trash,3,tx,ty);
  xsum=(tx*8.0);
  ysum=(ty*8.0);
  
  for (int dx=-4; dx<=4; dx++) {
    int x=xsum+dx;
    int y=ysum;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x,y, r, g, b);
  }
  for (int dy=-4; dy<=4; dy++) {
    int x=xsum;
    int y=ysum+dy;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img,x,y, r, g, b);
  }

}

void DrawTContour(Image& img, const Contours::Contour& c, unsigned int tx, unsigned int ty, unsigned int r, unsigned int g, unsigned int b)
{
  int xsum=0;
  int ysum=0;
  for (unsigned int i=0; i<c.size(); i++) {
    int x=c[i].first+tx;
    int y=c[i].second+ty;
    xsum+=x;
    ysum+=y;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x, y, r, g, b);
  }

  xsum /= c.size();
  ysum /= c.size();

  for (int dx=-10; dx<=10; dx++) {
    int x=xsum+dx;
    int y=ysum;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x,y, r, g, b);
  }
  for (int dy=-10; dy<=10; dy++) {
    int x=xsum;
    int y=ysum+dy;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img,x,y, r, g, b);
  }
 
  Contours::Contour trash;
  double ddx=0;
  double ddy=0;
  CenterAndReduce(c,trash,3,ddx,ddy);
  xsum=tx+(ddx*8.0);
  ysum=ty+(ddy*8.0);
  
  for (int dx=-4; dx<=4; dx++) {
    int x=xsum+dx;
    int y=ysum;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img, x,y, r, g, b);
  }
  for (int dy=-4; dy<=4; dy++) {
    int x=xsum;
    int y=ysum+dy;
    if (x >= 0 && x <= img.w && y >= 0 && y <= img.h)
      PutPixel(img,x,y, r, g, b);
  }

}
*/


bool WriteContour(FILE* f, const Contours::Contour& source)
{
  if (source.size() == 0) {
    if (fprintf(f, "! 0 0 0\n") < 0)
      return false;
  } else {
    unsigned int l=source.size();
    int lastx=source[0].first;
    int lasty=source[0].second;
    if (fprintf(f, "! %d %d %d\n", lastx, lasty, l) < 0)
      return false;

    int code=0;
    for (unsigned int i=1; i<l; i++) {
      int currentx=source[i].first;
      int currenty=source[i].second;
      int caddx=1+currentx-lastx;
      int caddy=1+currenty-lasty;
      assert(caddx >=0 && caddx < 3);
      assert(caddy >=0 && caddy < 3);
      int cadd=caddx+(3*caddy);
      lastx=currentx;
      lasty=currenty;
      
      if (i % 2 == 1)
	code=cadd;
      else {
	code+=3*3*cadd;
	if (fputc('"'+(char) code, f) == EOF)
	  return false;
      }
    }
    
    if (l % 2 == 0) {
      if (fputc('"'+(char) code, f)==EOF)
	return false;
    }
    if (fputc('\n', f) == EOF)
      return false;
  }

  return true;
}

bool ReadContour(FILE* f, Contours::Contour& dest)
{
  int l;
  int x;
  int y;
  if (fscanf(f, "! %d %d %d\n", &x, &y, &l) != 3)
    return false;
  dest.resize(l);
  if (l == 0)
    return true;
  dest[0].first=x;
  dest[0].second=y;
  int c=0;
  for (unsigned int i=1; i<(unsigned int) l ; i++) {
    if (i % 2 == 1) {
      c=fgetc(f);
      if (c == EOF)
	return false;
      c-='"';
    } else {
      c /= 3*3;
    }
    
    int dx=(c % 3) - 1;
    int dy=((c / 3) % 3) -1;
    x+=dx;
    y+=dy;
    dest[i].first=x;
    dest[i].second=y;
  }
  fgetc(f); // read linebreak;
  return true;
}


bool WriteContourArray(FILE* f, const std::vector <Contours::Contour*>& contours)
{
  unsigned int n=contours.size();
  if (fprintf(f, "CONTOURS v1 %d\n", n)<0)
    return false;
  for (unsigned int i=0; i<n; i++)
    if (!WriteContour(f,*(contours[i])))
      return false;
  return true;
}

bool ReadContourArray(FILE* f, std::vector <Contours::Contour*>& contours)
{
  unsigned int n=0;
  if (fscanf(f, "CONTOURS v1 %d\n", &n) != 1) {
    return false;
  }
  contours.resize(n);
  for (unsigned int i=0; i<n; i++) {
    contours[i]=new Contours::Contour;
    if (!ReadContour(f, *(contours[i]))) {
      for (unsigned int j=0; j<=i; j++)
	delete contours[j];
      contours.clear();
      return false;
    }
  }
  return true;
}
