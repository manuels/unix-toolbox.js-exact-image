/*
 * Copyright (C) 2007 - 2008 Lars Kuhtz, ExactCODE GmbH Germany.
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

#ifndef _SCANNER_HH_
#define _SCANNER_HH_

#include <map>
#include <vector>
#include <string>

#include "Tokenizer.hh"

namespace BarDecode
{

    // 16 bit should be enough we do not want to use all of these anyway.
    typedef uint16_t module_word_t;

    typedef uint32_t codes_t; // Bitset of types

    typedef unsigned int psize_t; // size in pixel type
    typedef unsigned int usize_t; // size in X unit

    typedef double u_t; // type for X unit

    enum code_t {
        ean8 = 1<<0,
        ean13 = 1<<1,
        upca = 1<<2,
        ean = ean8|ean13|upca,
        upce = 1<<3,
        code128 = 1<<4,
        gs1_128 = 1<<5,
        code39 = 1<<6,
        code39_mod43 = 1<<7,
        code39_ext = 1<<8,
        code25i = 1<<9,
        any_code = 0xfff
    };

    enum directions_t {
        left_right = 1<<0,
        top_down = 1<<1,
        right_left = 1<<2,
        down_top = 1<<3,
        any_direction = 0xf
    };

    std::ostream& operator<< (std::ostream& s, const code_t& t);

    struct bar_vector_t : public std::vector<token_t> 
    {
        bar_vector_t(int s) :
            std::vector<token_t>(s),
            bpsize(0),
            wpsize(0),
            psize(0)
        {}
        psize_t bpsize;
        psize_t wpsize;
        psize_t psize;
    };

    struct scanner_result_t
    {
        scanner_result_t() : 
            valid(false),
            type(),
            code(""),
            x(0),
            y(0)
        {}
        
        scanner_result_t(code_t type, const std::string& code, pos_t x, pos_t y) :
            valid(true),
            type(type),
            code(code),
            x(x),
            y(y)
        {}

        bool valid;
        code_t type;
        std::string code;
        pos_t x;
        pos_t y;

        operator bool() const
        {
            return valid;
        }
    };


    template<bool vertical = false>
    class BarcodeIterator
    {
    public:
        typedef BarcodeIterator self_t;
        typedef scanner_result_t value_type;
        typedef Tokenizer<vertical> tokenizer_t;
        typedef std::vector<token_t> token_line_t;

        static const psize_t min_quiet_psize = 7;

        BarcodeIterator(const Image* img,
                        threshold_t threshold = 150,
                        codes_t requested_codes = any_code, 
                        directions_t directions = any_direction,
                        int concurrent_lines = 4,
                        int line_skip = 8) :
            tokenizer(img,concurrent_lines,line_skip,threshold),
            requested_codes(requested_codes),
            directions(directions),
            cur_barcode(),
            token_line(),
            x(),
            y(),
            ti(),
            te()
        {
            if ( ! end() ) next();
        }

        virtual ~BarcodeIterator() {}

        value_type operator*() const
        {
            return cur_barcode;
        }

        // Try to find next modulizer
        self_t& operator++()
        {
            assert(! end());
            next();
            return *this;
        }

        bool end() const { return tokenizer.end(); }

    private:

        long pixel_diff(token_line_t::const_iterator a, const token_line_t::const_iterator& b)
        {
            long ret = 0;
            while( a != b ) {
                ret += (a++)->second;
            }
            return ret;
        }

        bool requested(code_t code) const
        {
            return code & requested_codes;
        }

        void next();

    private:
        tokenizer_t tokenizer;
        codes_t requested_codes;
        directions_t directions;
        scanner_result_t cur_barcode;
        token_line_t token_line;
        pos_t x, y;
        token_line_t::const_iterator ti,te;
    };

}; // namespace BarDecode

#include "Scanner.tcc"

#endif // _SCANNER_HH_
