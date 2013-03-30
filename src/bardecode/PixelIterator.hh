/*
 * Copyright (C) 2007 - 2008 Lars Kuhtz, ExactCODE GmbH Germany.
 * Copyright (C) 2009 - 2010 René Rebe, ExactCODE GmbH Germany.
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

#ifndef _PIXEL_ITERATOR_HH_
#define _PIXEL_ITERATOR_HH_

#include "Image.hh"

#include <iterator>
#include <vector>

#ifdef __APPLE__
#include <sys/types.h>
#endif

namespace BarDecode
{

    typedef int pos_t;
    typedef int threshold_t;

    template<bool vertical = false>
    class PixelIterator :
        public std::iterator<std::output_iterator_tag,
                             bool,
                             std::ptrdiff_t>
    {
    protected:
        typedef PixelIterator self_t;

    public:

        typedef bool value_type;

        PixelIterator(const Image* img, int concurrent_lines = 4, int line_skip = 8, threshold_t threshold = 0) :
            img(img),
            it_size(concurrent_lines),
            line_skip(line_skip),
            img_it(it_size),
            threshold(threshold),
            x(0),
            y(0),
            lum(0),
            valid_cache(false)
        {
            // FIXME insert an optimized code path for img->h <= it_size
            for (int i = 0; i < it_size; ++i) {
                img_it[i] = img->begin().at(0,std::min((int)i,img->h-1));
                *img_it[i];
            }
        }

        virtual ~PixelIterator() {};

        self_t& operator++()
        {
            valid_cache = false;
            if ( x < img->w-1 ) {
                ++x;
                for (int i = 0; i < it_size; ++i) {
                    ++img_it[i];
                    *img_it[i];
                }
            } else {
                x = 0;
                int todo = (img->h-1) - y;
                if ( todo > line_skip + (it_size-1) ) {
                    y += line_skip;
                    for (int i = 0; i < it_size; ++i) {
                        img_it[i] = img_it[i].at(x,y+(int)i);
                        *img_it[i];
                    }
                } else if ( todo <= line_skip ) {
                    // we are at the end
                    //++img_it[it_size-1];
                    img_it[it_size-1] = img->end();
                } else {
                    y += line_skip;
                    for (int i = 0; i < it_size; ++i) {
                        img_it[i] = img_it[i].at(x,std::min(y+(int)i,img->h-1));
                        *img_it[i];
                    }
                }
            }
            return *this;
        };

        // FIXME it seems that the median (or something similar) is the better choice.
        const value_type operator*() const
        {
            if (valid_cache) return cache;
            double tmp=0;
            //uint16_t min = 255;
            //uint16_t max = 0;
            for (int i = 0; i < it_size; ++i) {
                //min = std::min(min,img_it[i].getL());
                //max = std::max(max,img_it[i].getL());
                tmp += img_it[i].getL();
            }
            //lum = (tmp - (double) (min+max)) / (double) (it_size-2);
            lum = tmp / it_size;
            cache = lum < threshold;
            valid_cache = true;
            return cache;
        }
            
        //value_type* operator->();
        //const value_type* operator->() const;

        self_t at(pos_t x, pos_t y) const
        {
            // FIXME insert an optimized code path for img->h >= y+it_size
            self_t tmp = *this;
            for (int i = 0; i < it_size; ++i) {
                tmp.img_it[i] = tmp.img_it[i].at(x,std::min(y+(int)i,img->h));
            }
            tmp.valid_cache = false;
            tmp.x = x;
            tmp.y = y;
            return tmp;
        }

        pos_t get_x() const { return x; }
        pos_t get_y() const { return y; }

        threshold_t get_threshold() const { return threshold; }

        void set_threshold(threshold_t new_threshold) 
        {
            valid_cache = false;
            threshold = new_threshold; 
        }

        bool end() const { return !(img_it[it_size-1] != img->end()); }

        double get_lum() const 
        {
            if (! valid_cache) {
                operator*();
            }
            return lum;
        }

        long get_x_size() const { return img->w; }
        long get_y_size() const { return img->h; }
        long get_line_length() const { return get_x_size(); }

    protected:
        const Image* img;
        int it_size;
        int line_skip;
        std::vector<Image::const_iterator> img_it;
        threshold_t threshold;
        pos_t x;
        pos_t y;
        mutable double lum;
        mutable bool cache;
        mutable bool valid_cache;
    }; // class PixelIterator<vertical = true>

    // vertical iteration
    template<>
    class PixelIterator<true> : 
        public std::iterator<std::output_iterator_tag,
                             bool,
                             std::ptrdiff_t>
    {
    protected:
        typedef PixelIterator self_t;

    public:

        typedef bool value_type;

        PixelIterator(const Image* img, int concurrent_lines = 4, int line_skip = 8, threshold_t threshold = 0) :
            img(img),
            it_size(concurrent_lines),
            line_skip(line_skip),
            img_it(it_size),
            threshold(threshold),
            x(0),
            y(0),
            lum(0),
            valid_cache(false)
        {
            // FIXME insert an optimized code path for img->h <= it_size
            for (int i = 0; i < it_size; ++i) {
                img_it[i] = img->begin().at(std::min((int)i,img->w-1),0);
                *img_it[i];
            }
        }

        virtual ~PixelIterator() {};

        self_t& operator++()
        {
            valid_cache = false;
            if ( y < img->h-1 ) {
                ++y;
                for (int i = 0; i < it_size; ++i) {
                    img_it[i].down();
                    *img_it[i];
                }
            } else {
                y = 0;
                int todo = (img->w-1) - x;
                if ( todo > line_skip + (it_size-1) ) {
                    x += line_skip;
                    for (int i = 0; i < it_size; ++i) {
                        img_it[i] = img_it[i].at(x+(int) i,y);
                        *img_it[i];
                    }
                } else if ( todo <= line_skip ) {
                    img_it[it_size-1] = img->end();
                } else {
                    x += line_skip;
                    for (int i = 0; i < it_size; ++i) {
                        img_it[i] = img_it[i].at(std::min(x+(int)i,img->w-1),y);
                        *img_it[i];
                    }
                }
            }
            return *this;
        };

        // FIXME it seems that the median (or something similar) is the better choice.
        const value_type operator*() const
        {
            if (valid_cache) return cache;
            double tmp=0;
            //uint16_t min = 255;
            //uint16_t max = 0;
            for (int i = 0; i < it_size; ++i) {
                //min = std::min(min,img_it[i].getL());
                //max = std::max(max,img_it[i].getL());
                tmp += img_it[i].getL();
            }
            //lum = (tmp - (double) (min+max)) / (double) (it_size-2);
            lum = tmp / it_size;
            cache = lum < threshold;
            valid_cache = true;
            return cache;
        }
            
        //value_type* operator->();
        //const value_type* operator->() const;

        self_t at(pos_t x, pos_t y) const
        {
            // FIXME insert an optimized code path for img->h >= y+it_size
            self_t tmp = *this;
            for (int i = 0; i < it_size; ++i) {
                tmp.img_it[i] = tmp.img_it[i].at(std::min(x+(int)i,img->w-1),y);
            }
            tmp.valid_cache = false;
            tmp.x = x;
            tmp.y = y;
            return tmp;
        }

        pos_t get_x() const { return x; }
        pos_t get_y() const { return y; }

        threshold_t get_threshold() const { return threshold; }

        void set_threshold(threshold_t new_threshold) 
        {
            valid_cache = false;
            threshold = new_threshold; 
        }

        bool end() const { return !(img_it[it_size-1] != img->end()); }

        double get_lum() const 
        {
            if (! valid_cache) {
                operator*();
            }
            return lum;
        }

        long get_x_size() const { return img->w; }
        long get_y_size() const { return img->h; }
        long get_line_length() const { return get_y_size(); }

    protected:
        const Image* img;
        int it_size;
        int line_skip;
        std::vector<Image::const_iterator> img_it;
        threshold_t threshold;
        pos_t x;
        pos_t y;
        mutable double lum;
        mutable bool cache;
        mutable bool valid_cache;

    }; // class PixelIterator<vertical = true>

}; // namespace BarDecode

#endif // _PIXEL_ITERATOR_HH_
