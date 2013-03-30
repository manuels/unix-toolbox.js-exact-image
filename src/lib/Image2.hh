#ifndef IMAGE2_HH
#define IMAGE2_HH

#include <inttypes.h>

// the more optimizing next-gen Image::iterator DRAFT

#include "Image.hh" // the Plain Old Data image

/*
  Colorspace: GRAY, GRAYA, RGB, RGBA, CMYK, YUV, YUVA
  Bit depth:  INT1 INT2 INT4        INT8 INT16 INT32 FLOAT32 FLOAT64
*/

/*
  Operations:
  
  begin
  end
  ++
  --
  at
  +=
  -=
  /=
*/

typedef enum {
  GRAY,
  GRAYA,
  RGB,
  RGBA,
  CMYK,
  YUV,
} type_t;

template<class T, class bps>
class iterator2
  {
  public:
    Image* image;
    
    int stride, width, _x;
    ivalue_t value;

    value_t* ptr;
    signed int bitpos; // for 1bps sub-position
    
    // for seperate use, e.g. to accumulate
    iterator2 ()
    {};
    
    iterator2 (Image* _image, bool end)
      : image (_image), type (_image->Type()),
	stride (_image->Stride()), width (image->w)
    {
      if (!end) {
	ptr = (value_t*) image->data;
	_x = 0;
	bitpos = 7;
      }
      else {
	ptr = (value_t*) (image->data + stride * image->h);
	_x = width;
	// TODO: bitpos= ...
      }
    }
    
    inline void clear () {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray = 0;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r = value.rgb.g = value.rgb.b = 0;
	break;
      case CMYK8:
	value.cmyk.c = value.cmyk.m = value.cmyk.y = value.cmyk.k = 0;
	break;
      case YUV8:
	value.yuv.y = value.yuv.u = value.yuv.v = 0;
	break;
      }
    }
    
    inline iterator2 at (int x, int y) {
      iterator2 tmp = *this;
      
      switch (type) {
      case GRAY1:
	tmp.ptr = (value_t*) (image->data + stride * y + x / 8);
	tmp.bitpos = 7 - x % 8;
	break;
      case GRAY2:
	tmp.ptr = (value_t*) (image->data + stride * y + x / 4);
	tmp.bitpos = 7 - (x % 4) * 2;
	break;
      case GRAY4:
	tmp.ptr = (value_t*) (image->data + stride * y + x / 2);
	tmp.bitpos = 7 - (x % 2) * 4;
	break;
      case GRAY8:
	tmp.ptr = (value_t*) (image->data + stride * y + x);
	break;
      case GRAY16:
	tmp.ptr = (value_t*) (image->data + stride * y + x * 2);
	break;
      case RGB8:
      case YUV8:
	tmp.ptr = (value_t*) (image->data + stride * y + x * 3);
	break;
      case RGB16:
	tmp.ptr = (value_t*) (image->data + stride * y + x * 6);
	break;
      case CMYK8:
	tmp.ptr = (value_t*) (image->data + stride * y + x * 4);
	break;
      }
      return tmp;
    }

    inline iterator2& operator* () {
      switch (type) {
      case GRAY1:
	value.gray = (ptr->gray >> (bitpos-0) & 0x01) * 255;
	break;
      case GRAY2:
	value.gray = (ptr->gray >> (bitpos-1) & 0x03) * 255/3;
	break;
      case GRAY4:
	value.gray = (ptr->gray >> (bitpos-3) & 0x0f) * 255/15;
	break;
      case GRAY8:
	value.gray = ptr->gray;
	break;
      case GRAY16:
	value.gray = ptr->gray16;
	break;
      case RGB8:
	value.rgb.r = ptr->rgb.r;
	value.rgb.g = ptr->rgb.g;
	value.rgb.b = ptr->rgb.b;
	break;
      case RGB16:
	value.rgb.r = ptr->rgb16.r;
	value.rgb.g = ptr->rgb16.g;
	value.rgb.b = ptr->rgb16.b;
	break;
      case CMYK8:
	value.cmyk.c = ptr->cmyk.c;
	value.cmyk.m = ptr->cmyk.m;
	value.cmyk.y = ptr->cmyk.y;
	value.cmyk.k = ptr->cmyk.k;
	break;
      case YUV8:
	value.yuv.y = ptr->yuv.y;
	value.yuv.u = ptr->yuv.u;
	value.yuv.v = ptr->yuv.v;
	break;
      }
      return *this;
    }
    
    inline iterator2& operator+= (const iterator2& other) {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray += other.value.gray;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r += other.value.rgb.r;
	value.rgb.g += other.value.rgb.g;
	value.rgb.b += other.value.rgb.b;
	break;
      case CMYK8:
	value.cmyk.c += other.value.cmyk.c;
	value.cmyk.m += other.value.cmyk.m;
	value.cmyk.y += other.value.cmyk.y;
	value.cmyk.k += other.value.cmyk.k;
	break;
      case YUV8:
	value.yuv.y += other.value.yuv.y;
	value.yuv.u += other.value.yuv.u;
	value.yuv.v += other.value.yuv.v;
	break;     
      }
      return *this;
    }
    
    inline iterator2 operator+ (const iterator2& other) const {
      iterator2 tmp = *this;
      return tmp += other;
    }
    
    inline iterator2 operator+ (int v) const {
      iterator2 tmp = *this;
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	tmp.value.gray += v;
	break;
      case RGB8:
      case RGB16:
	tmp.value.rgb.r += v;
	tmp.value.rgb.g += v;
	tmp.value.rgb.b += v;
	break;
      case CMYK8:
	tmp.value.cmyk.c += v;
	tmp.value.cmyk.m += v;
	tmp.value.cmyk.y += v;
	tmp.value.cmyk.k += v;
	break;
      case YUV8:
	tmp.value.yuv.y += v;
	tmp.value.yuv.u += v;
	tmp.value.yuv.v += v;
	break;     
      }
      return tmp;
    }
    
    inline iterator2& operator-= (const iterator2& other)  {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray -= other.value.gray;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r -= other.value.rgb.r;
	value.rgb.g -= other.value.rgb.g;
	value.rgb.b -= other.value.rgb.b;
	break;
      case CMYK8:
	value.cmyk.c -= other.value.cmyk.c;
	value.cmyk.m -= other.value.cmyk.m;
	value.cmyk.y -= other.value.cmyk.y;
	value.cmyk.k -= other.value.cmyk.k;
	break;
      case YUV8:
	value.yuv.y -= other.value.yuv.y;
	value.yuv.u -= other.value.yuv.u;
	value.yuv.v -= other.value.yuv.v;
	break;
      }
      return *this;
    }
    
    inline iterator2& operator- (const iterator2& other) const {
      iterator2 tmp = *this;
      return tmp -= other;
    }
    
    inline iterator2& operator*= (const int v) {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray *= v;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r *= v;
	value.rgb.g *= v;
	value.rgb.b *= v;
	break;
      case CMYK8:
	value.cmyk.c *= v;
	value.cmyk.m *= v;
	value.cmyk.y *= v;
	value.cmyk.k *= v;
	break;
      case YUV8:
	value.yuv.y *= v;
	value.yuv.u *= v;
	value.yuv.v *= v;
	break;
      }
      return *this;
    }
    
    inline iterator2 operator* (const int v) const {
      iterator2 tmp = *this;
      return tmp *= v;
    }
    
    inline iterator2& operator/= (const int v) {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray /= v;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r /= v;
	value.rgb.g /= v;
	value.rgb.b /= v;
	break;
      case CMYK8:
	value.cmyk.c /= v;
	value.cmyk.m /= v;
	value.cmyk.y /= v;
	value.cmyk.k /= v;
	break;
      case YUV8:
	value.yuv.y /= v;
	value.yuv.u /= v;
	value.yuv.v /= v;
	break;
      }
      return *this;
    }

    inline iterator2& operator/ (const int v) const {
      iterator2 tmp = *this;
      return tmp /= v;
    }
    
    inline iterator2& limit () {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	if (value.gray > 0xff)
	  value.gray = 0xff;
	break;
      case RGB8:
	if (value.rgb.r > 0xff)
	  value.rgb.r = 0xff;
	if (value.rgb.g > 0xff)
	  value.rgb.g = 0xff;
	if (value.rgb.b > 0xff)
	  value.rgb.b = 0xff;
	break;
      case RGB16:
	if (value.rgb.r > 0xffff)
	  value.rgb.r = 0xffff;
	if (value.rgb.g > 0xffff)
	  value.rgb.g = 0xffff;
	if (value.rgb.b > 0xffff)
	  value.rgb.b = 0xffff;
	break;
      }
      return *this;
    }
    
    //prefix
    inline iterator2& operator++ () {
      switch (type) {
      case GRAY1:
	--bitpos; ++_x;
	if (bitpos < 0 || _x == width) {
	  bitpos = 7;
	  if (_x == width)
	    _x = 0;
	  ptr = (value_t*) ((uint8_t*) ptr + 1);
	}
	break;
      case GRAY2:
	bitpos -= 2; ++_x;
	if (bitpos < 0 || _x == width) {
	  bitpos = 7;
	  if (_x == width)
	    _x = 0;
	  ptr = (value_t*) ((uint8_t*) ptr + 1);
	}
	break;
      case GRAY4:
	bitpos -= 4; ++_x;
	if (bitpos < 0 || _x == width) {
	  bitpos = 7;
	  if (_x == width)
	    _x = 0;
	  ptr = (value_t*) ((uint8_t*) ptr + 1);
	}
	break;
      case GRAY8:
	ptr = (value_t*) ((uint8_t*) ptr + 1);
	break;
      case GRAY16:
	ptr = (value_t*) ((uint8_t*) ptr + 2); break;
      case RGB8:
      case YUV8:
	ptr = (value_t*) ((uint8_t*) ptr + 3); break;
      case RGB16:
	ptr = (value_t*) ((uint8_t*) ptr + 6); break;
      case CMYK8:
	ptr = (value_t*) ((uint8_t*) ptr + 4); break;
      }
      return *this;
    }
    
    inline iterator2& operator-- () {
      switch (type) {
      case GRAY1:
	++bitpos; --_x;
	if (bitpos > 7) {
	  bitpos = 0;
	  ptr = (value_t*) ((uint8_t*) ptr - 1);
	}
	break;
      case GRAY2:
	bitpos += 2; --_x;
	if (bitpos > 7) {
	  bitpos = 1;
	  ptr = (value_t*) ((uint8_t*) ptr - 1);
	}
	break;
      case GRAY4:
	bitpos += 4; --_x;
	if (bitpos > 7) {
	  bitpos = 3;
	  ptr = (value_t*) ((uint8_t*) ptr - 1);
	}
	break;
      case GRAY8:
	ptr = (value_t*) ((uint8_t*) ptr - 1); break;
      case GRAY16:
	ptr = (value_t*) ((uint8_t*) ptr - 2); break;
      case RGB8:
      case YUV8:
	ptr = (value_t*) ((uint8_t*) ptr - 3); break;
      case RGB16:
	ptr = (value_t*) ((uint8_t*) ptr - 6); break;
      case CMYK8:
	ptr = (value_t*) ((uint8_t*) ptr - 4); break;
      }
      return *this;
    }
    
    // return Luminance
    inline uint16_t getL ()
    {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	return value.gray;
	break;
      case RGB8:
      case RGB16:
	return (uint16_t) (.21267 * value.rgb.r +
			    .71516 * value.rgb.g +
			    .07217 * value.rgb.b);
	break;
      case CMYK8:
	return value.cmyk.k; // TODO
	break;
      case YUV8:
	return value.yuv.y;
	break;
      }
    }
    
    // return Luminance
    inline void getRGB(uint16_t* r, uint16_t* g, uint16_t* b)
    {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	*r = *g = *b = value.gray;
	return;
	break;
      case RGB8:
      case RGB16:
	*r = value.rgb.r;
	*g = value.rgb.g;
	*b = value.rgb.b;
	return;
	break;
      case CMYK8:
	// TODO
	break;
      case YUV8:
	// TODO
	break;
      }
    }
    
    // set Luminance
    inline void setL (uint16_t L)
    {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray = L;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r = value.rgb.g = value.rgb.b = L;
	break;
      case CMYK8:
	// TODO:
	value.cmyk.c = value.cmyk.m = value.cmyk.y = 0;
	value.cmyk.k = L;
	break;
      case YUV8:
	value.yuv.u = value.yuv.v = 0;
	value.yuv.y = L;
 	break;
      }
    }
    
    // set RGB
    inline void setRGB(uint16_t r, uint16_t g, uint16_t b)
    {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
      case GRAY8:
      case GRAY16:
	value.gray = (int) (.21267 * r + .71516 * g + .07217 * b);
	return;
	break;
      case RGB8:
      case RGB16:
	value.rgb.r = r;
	value.rgb.g = g;
	value.rgb.b = b;
	return;
	break;
      case CMYK8:
	// TODO
	break;
      case YUV8:
	// TODO
	break;
      }
    }
    
    inline void set (const iterator2& other) {
      switch (type) {
      case GRAY1:
	ptr->gray |= (other.value.gray >> 7) << bitpos;
	break;
      case GRAY2:
	ptr->gray |= (other.value.gray >> 6) << (bitpos-1);
	break;
      case GRAY4:
	ptr->gray |= (other.value.gray >> 4) << (bitpos-3);
	break;
      case GRAY8:
	ptr->gray = other.value.gray;
	break;
      case GRAY16:
	ptr->gray16 = other.value.gray;
	break;
      case RGB8:
	ptr->rgb.r = other.value.rgb.r;
	ptr->rgb.g = other.value.rgb.g;
	ptr->rgb.b = other.value.rgb.b;
	break;
      case RGB16:
	ptr->rgb16.r = other.value.rgb.r;
	ptr->rgb16.g = other.value.rgb.g;
	ptr->rgb16.b = other.value.rgb.b;
	break;
      case CMYK8:
	ptr->cmyk.c = other.value.cmyk.c;
	ptr->cmyk.m = other.value.cmyk.m;
	ptr->cmyk.y = other.value.cmyk.y;
	ptr->cmyk.k = other.value.cmyk.k;
	break;
      case YUV8:
	ptr->yuv.y = other.value.yuv.y;
	ptr->yuv.u = other.value.yuv.u;
	ptr->yuv.v = other.value.yuv.v;
	break;
      }
    }
    
    bool operator != (const iterator2& other)
    {
      switch (type) {
      case GRAY1:
      case GRAY2:
      case GRAY4:
	return ptr != other.ptr && _x != other._x;
      case GRAY8:
      case GRAY16:
      case RGB8:
      case RGB16:
      case CMYK8:
      case YUV8:
	return ptr != other.ptr;
      }

    }
  };

#endif
