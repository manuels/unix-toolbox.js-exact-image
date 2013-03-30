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

#ifndef _TOKENIZER_HH_
#define _TOKENIZER_HH_

#include <utility>
#include <assert.h>

#include <math.h>

#include <algorithm>

#include "Image.hh"
#include "PixelIterator.hh"

namespace BarDecode
{

    // Shall we include absolut position?
    typedef std::pair<bool,uint> token_t; // (color,length in pixels)
    typedef bool module_t;
    typedef double unit_t;

    template<bool vertical = false>
    class Tokenizer
    {
    public:
        Tokenizer(const Image* img, int concurrent_lines = 4, int line_skip = 8, threshold_t threshold = 150) :
            img(img),
            it(img,concurrent_lines,line_skip,threshold),
            extra(0),
            initial_threshold(threshold)
        {}

        virtual ~Tokenizer() {}

        void next_line(std::vector<token_t>& result)
        {
            assert(! end());
            assert(vertical ? it.get_y() == 0 : it.get_x() == 0);

            result.clear();

            it.set_threshold(initial_threshold);

            //std::cerr << "========== 0 " << it.get_threshold() <<  std::endl;

            bool color = *it;
            double count = 1;
            double lum;

#define DYNAMIC_THRESHOLD
#ifdef DYNAMIC_THRESHOLD
            double sum = 0;
#endif

            double old_mean = it.get_lum();

            for (long i = 0; i < it.get_line_length(); ++i,++it,++count) {

                lum = it.get_lum();
#ifdef DYNAMIC_THRESHOLD
                sum += lum;
#endif

#ifdef DYNAMIC_THRESHOLD
                double mean =  sum / count;
                //std::cerr << "---------------- lum=" << lum << " sum=" << sum << " count=" << count << " mean=" << mean << std::endl;
                static const int contrast_threshold = 30;
                if ( ! color && lum > it.get_threshold() && lum < mean - contrast_threshold) {
                    //it.set_threshold(lround(std::max((double)it.get_threshold(),std::min((mean - contrast_threshold)/2,220.0))));
                    it.set_threshold(lround(std::min((mean - contrast_threshold),220.0)));
                } else if ( ! color && lum > it.get_threshold() && (old_mean && lum < old_mean - (contrast_threshold+10))) {
                    it.set_threshold(lround(std::min((old_mean - (contrast_threshold+10)),220.0)));
                } else if ( color && lum < it.get_threshold() && (lum > mean + contrast_threshold)) {
                    //it.set_threshold(lround(std::min((double)it.get_threshold(),std::max((mean + contrast_threshold)/2,80.0))));
                    it.set_threshold(lround(std::max((mean + contrast_threshold),80.0)));
                } else if ( color && lum < it.get_threshold() && (old_mean && lum > old_mean + contrast_threshold+10)) {
                    it.set_threshold(lround(std::max((old_mean + contrast_threshold+10),80.0)));
                } 
#if 1
                //else if ( color && count > 20 && lum < it.get_threshold() && lum < mean - contrast_threshold && it.get_threshold() > initial_threshold) {
                else if ( color && count > 20 && lum < it.get_threshold() && lum < mean - contrast_threshold /*&& it.get_threshold() > initial_threshold*/) {
                    color = false;
                    it.set_threshold(lround(mean - contrast_threshold));
                    //it.set_threshold(initial_threshold);
                } else if ( color && count > 20 && lum < it.get_threshold() && lum < old_mean - (contrast_threshold+10) /*&& it.get_threshold() > initial_threshold*/) {
                    color = false;
                    it.set_threshold(lround(old_mean - (contrast_threshold+10)));
                    //it.set_threshold(initial_threshold);
                }
                old_mean = mean;
#endif
                //mean = ( *it ? std::min(mean,lum) : std::max(mean,lum));
#endif

                if (color != *it || i == it.get_line_length()-1) {

#define SUBPIXEL_ADJUST
#ifdef SUBPIXEL_ADJUST
                    count -= extra;
                    double extra = ( ! color ?  (lum / 255.0) : (1- (lum / 255.0)));
                    count += extra;
                    extra = 1 - extra;
#endif
                    result.push_back(token_t(color,lround(count)));
#define DYNAMIC_THRESHOLD
#ifdef DYNAMIC_THRESHOLD
                    sum = 0;
#endif
                    count = 0;
                    color = *it; 
                }
            }
        }



        // precondition: ! end()
        // FIXME use extra only if u is small enough such that subpixel dimensions matter
        token_t next()
        {
            assert(! end());

#define DYNAMIC_THRESHOLD
#ifdef DYNAMIC_THRESHOLD
            double sum = 0;
#endif

            double count = 0;
            bool color = *it; // TODO simple alternation would safe the call of operator*
            double lum = it.get_lum();
            //double mean = lum;
            do { 

#ifdef DYNAMIC_THRESHOLD
                sum += lum;
#endif

                ++count;
                ++it; 
                lum = it.get_lum();

#ifdef DYNAMIC_THRESHOLD
                double mean =  sum / count;
                static const int contrast_threshold = 50;
                if ( ! color && lum > it.get_threshold() && lum < mean - contrast_threshold) {
                    // std::cerr << "0 adjust from " << it.get_threshold() << " to ";
                    //it.set_threshold(lround(std::max((double)it.get_threshold(),std::min((mean - contrast_threshold)/2,220.0))));
                    it.set_threshold(lround(std::min((mean - contrast_threshold),220.0)));
                    //std::cerr << it.get_threshold() << std::endl;
                    break;
                } else if ( color && lum < it.get_threshold() && lum > mean + contrast_threshold) {
                    //std::cerr << "1 adjust from " << it.get_threshold() << " to ";
                    //it.set_threshold(lround(std::min((double)it.get_threshold(),std::max((mean + contrast_threshold)/2,80.0))));
                    it.set_threshold(lround(std::max((mean + contrast_threshold),80.0)));
                    //std::cerr << it.get_threshold() << std::endl;
                    break;
                } 
#if 1
                else if ( color && count > 10 && lum < it.get_threshold() && lum < mean - contrast_threshold /* && it.get_threshold() > initial_threshold*/) {
                    color = false;
                    // std::cerr << "2 adjust from " << it.get_threshold() << " to ";
                    it.set_threshold(lround(mean - contrast_threshold));
                    //it.set_threshold(initial_threshold);
                    //std::cerr << it.get_threshold() << std::endl;
                    //break;
                }
#endif
                //mean = ( *it ? std::min(mean,lum) : std::max(mean,lum));
#endif

            } while ( ! end() && color == *it );

#define SUBPIXEL_ADJUST
#ifdef SUBPIXEL_ADJUST
            count -= extra;
            double extra = ( ! color ?  (lum / 255.0) : (1- (lum / 255.0)));
            count += extra;
            extra = 1 - extra;
#endif
            return token_t(color,lround(count));
        }

        bool end() const { return it.end(); }

        threshold_t get_threshold() const { return it.get_threshold(); }
        void set_threshold(threshold_t new_threshold) 
        { 
            it.set_threshold(new_threshold); 
        }

        pos_t get_x() const { return it.get_x(); }
        pos_t get_y() const { return it.get_y(); }
        const Image* get_img() const { return img; }

        Tokenizer at(pos_t x, pos_t y) const
        {
            Tokenizer tmp = *this;
            tmp.it = it.at(x,y);
            return tmp;
        }

    protected:
        const Image* img;
        PixelIterator<vertical> it;
#ifdef SUBPIXEL_ADJUST
        double extra;
#endif
        threshold_t initial_threshold;
    };

}; // namespace BarDecode

#endif // _TOKENIZER_HH_
