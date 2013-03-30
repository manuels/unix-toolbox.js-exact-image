/*
 * Copyright (C) 2007 - 2008 Lars Kuhtz, ExactCODE GmbH Germany.
 * Copyright (C) 2010 René Rebe, ExactCODE GmbH Germany.
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

#ifndef _CODE39_HH_
#define _CODE39_HH_

#include "scanner_utils.hh"

namespace BarDecode
{
    struct code39_t
    {
        code39_t();

        // NOTE: if the first char is SHIFT, then a further barcode is expected to
        // be concatenated.

        // Character set: A-Z0-9-. $/+% and DELIMITER

        // Extended set: full ascii (by use of shift characters)
        // Usage of extended set is not encoded, but needs to be requested explicitly.
        // The code is first scaned normaly. The result is translated afterwards
        // into extend encoding

        // Usage of checksum is not encoded, but needs to be requested explicitly.
        // If requested this is performed after completely having scanned the code.


        // Decoding is based on a binary encoding of the width of bars (rather than
        // modules). Since each symbol has 9 bars we need a table of size 512.
        // Otherwith we would have needed size 2^(13 modules - 2 constant modules) = 2048.
        // ((Maybe we could safe even a bit more by directly encoding 3 of 9 ???)

        template<class TIT>
        scanner_result_t scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;

        template<class TIT>
        scanner_result_t reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;

        bool check_bar_vector(const bar_vector_t& b,psize_t old_psize = 0) const;
        module_word_t get_key(const bar_vector_t& b) const;
        module_word_t reverse_get_key(const bar_vector_t& b) const;
        
        template<class TIT>
        bool expect_n(TIT& start, TIT end, psize_t old_psize) const;

        static const char DELIMITER  = 254;
        static const char no_entry = 255;

        static const usize_t min_quiet_usize = 5;
        //static const usize_t min_quiet_usize = 10;
        static const usize_t min_quiet_usize_right = 5;

        DECLARE_TABLE(table,512);
        DECLARE_TABLE(aux,128);
    };

    inline code39_t::code39_t()
    {
        INIT_TABLE(table,512,no_entry);
        PUT_IN_TABLE(table,0x34, '0');
        PUT_IN_TABLE(table,0x121, '1');
        PUT_IN_TABLE(table,0x61, '2');
        PUT_IN_TABLE(table,0x160, '3');
        PUT_IN_TABLE(table,0x31, '4');
        PUT_IN_TABLE(table,0x130, '5');
        PUT_IN_TABLE(table,0x70, '6');
        PUT_IN_TABLE(table,0x25, '7');
        PUT_IN_TABLE(table,0x124, '8');
        PUT_IN_TABLE(table,0x64, '9');
        PUT_IN_TABLE(table,0x109, 'A');
        PUT_IN_TABLE(table,0x49, 'B');
        PUT_IN_TABLE(table,0x148, 'C');
        PUT_IN_TABLE(table,0x19, 'D');
        PUT_IN_TABLE(table,0x118, 'E');
        PUT_IN_TABLE(table,0x58, 'F');
        PUT_IN_TABLE(table,0xD, 'G');
        PUT_IN_TABLE(table,0x10C, 'H');
        PUT_IN_TABLE(table,0x4C, 'I');
        PUT_IN_TABLE(table,0x1C, 'J');
        PUT_IN_TABLE(table,0x103, 'K');
        PUT_IN_TABLE(table,0x43, 'L');
        PUT_IN_TABLE(table,0x142, 'M');
        PUT_IN_TABLE(table,0x13, 'N');
        PUT_IN_TABLE(table,0x112, 'O');
        PUT_IN_TABLE(table,0x52, 'P');
        PUT_IN_TABLE(table,0x7, 'Q');
        PUT_IN_TABLE(table,0x106, 'R');
        PUT_IN_TABLE(table,0x46, 'S');
        PUT_IN_TABLE(table,0x16, 'T');
        PUT_IN_TABLE(table,0x181, 'U');
        PUT_IN_TABLE(table,0xC1, 'V');
        PUT_IN_TABLE(table,0x1C0, 'W');
        PUT_IN_TABLE(table,0x91, 'X');
        PUT_IN_TABLE(table,0x190, 'Y');
        PUT_IN_TABLE(table,0xD0, 'Z');
        PUT_IN_TABLE(table,0x85, '-');
        PUT_IN_TABLE(table,0x184, '.');
        PUT_IN_TABLE(table,0xC4, ' ');
        PUT_IN_TABLE(table,0xA8, '$');
        PUT_IN_TABLE(table,0xA2, '/');
        PUT_IN_TABLE(table,0x8A, '+');
        PUT_IN_TABLE(table,0x2A, '%');
        PUT_IN_TABLE(table,0x94, DELIMITER);

        INIT_TABLE(aux,128,no_entry);
        PUT_IN_TABLE(aux,(uint)'0', 0);
        PUT_IN_TABLE(aux,(uint)'1', 1);
        PUT_IN_TABLE(aux,(uint)'2', 2);
        PUT_IN_TABLE(aux,(uint)'3', 3);
        PUT_IN_TABLE(aux,(uint)'4', 4);
        PUT_IN_TABLE(aux,(uint)'5', 5);
        PUT_IN_TABLE(aux,(uint)'6', 6);
        PUT_IN_TABLE(aux,(uint)'7', 7);
        PUT_IN_TABLE(aux,(uint)'8', 8);
        PUT_IN_TABLE(aux,(uint)'9', 9);
        PUT_IN_TABLE(aux,(uint)'A', 10);
        PUT_IN_TABLE(aux,(uint)'B', 11);
        PUT_IN_TABLE(aux,(uint)'C', 12);
        PUT_IN_TABLE(aux,(uint)'D', 13);
        PUT_IN_TABLE(aux,(uint)'E', 14);
        PUT_IN_TABLE(aux,(uint)'F', 15);
        PUT_IN_TABLE(aux,(uint)'G', 16);
        PUT_IN_TABLE(aux,(uint)'H', 17);
        PUT_IN_TABLE(aux,(uint)'I', 18);
        PUT_IN_TABLE(aux,(uint)'J', 19);
        PUT_IN_TABLE(aux,(uint)'K', 20);
        PUT_IN_TABLE(aux,(uint)'L', 21);
        PUT_IN_TABLE(aux,(uint)'M', 22);
        PUT_IN_TABLE(aux,(uint)'N', 23);
        PUT_IN_TABLE(aux,(uint)'O', 24);
        PUT_IN_TABLE(aux,(uint)'P', 25);
        PUT_IN_TABLE(aux,(uint)'Q', 26);
        PUT_IN_TABLE(aux,(uint)'R', 27);
        PUT_IN_TABLE(aux,(uint)'S', 28);
        PUT_IN_TABLE(aux,(uint)'T', 29);
        PUT_IN_TABLE(aux,(uint)'U', 30);
        PUT_IN_TABLE(aux,(uint)'V', 31);
        PUT_IN_TABLE(aux,(uint)'W', 32);
        PUT_IN_TABLE(aux,(uint)'X', 33);
        PUT_IN_TABLE(aux,(uint)'Y', 34);
        PUT_IN_TABLE(aux,(uint)'Z', 35);
        PUT_IN_TABLE(aux,(uint)'-', 36);
        PUT_IN_TABLE(aux,(uint)'.', 37);
        PUT_IN_TABLE(aux,(uint)' ', 38);
        PUT_IN_TABLE(aux,(uint)'$', 39);
        PUT_IN_TABLE(aux,(uint)'/', 40);
        PUT_IN_TABLE(aux,(uint)'+', 41);
        PUT_IN_TABLE(aux,(uint)'%', 42);
    }

    inline module_word_t code39_t::get_key(const bar_vector_t& b) const
    {
#ifdef STRICT
        u_t n_l = ((double) b.psize / 15.0); // ((b.size / (6*1+3*3)) * 1
        u_t n_h = ((double) b.psize / 12.0); // ((b.size / (6*1+3*2)) * 1
        u_t w_l = ((double) b.psize / 6.0);  // ((b.size / (6*1+3*2)) * 2
        u_t w_h = ((double) b.psize / 5.0);   // ((b.size / (6*1+3*3)) * 3
#else
        u_t n_l = ((double) b.psize / 30.0);
        u_t n_h = ((double) b.psize / 8.0);
        u_t w_l = ((double) b.psize / 7.9);
        u_t w_h = ((double) b.psize / 1.0);

#endif
        assert(b.size() == 9);
        module_word_t r = 0;
        for (uint i = 0; i < 9; ++i) {
            r <<= 1;
            if (w_l <= b[i].second && b[i].second <= w_h) r += 1;
            else if (! (n_l <= b[i].second && b[i].second <= n_h)) return 0;
        }
        return r;
    }

    inline module_word_t code39_t::reverse_get_key(const bar_vector_t& b) const
    {
#ifdef STRICT
        u_t n_l = ((double) b.psize / 15.0); // ((b.size / (6*1+3*3)) * 1
        u_t n_h = ((double) b.psize / 12.0); // ((b.size / (6*1+3*2)) * 1
        u_t w_l = ((double) b.psize / 6.0);  // ((b.size / (6*1+3*2)) * 2
        u_t w_h = ((double) b.psize / 5.0);   // ((b.size / (6*1+3*3)) * 3
#else
        u_t n_l = ((double) b.psize / 30.0);
        u_t n_h = ((double) b.psize / 8.0);
        u_t w_l = ((double) b.psize / 7.9);
        u_t w_h = ((double) b.psize / 1.0);

#endif
        assert(b.size() == 9);
        module_word_t r = 0;
        for (int i = 8; i >= 0; --i) {
            r <<= 1;
            if (w_l <= b[i].second && b[i].second <= w_h) r += 1;
            else if (! (n_l <= b[i].second && b[i].second <= n_h)) return 0;
        }
        return r;
    }

    // psize = 0 means skip that test
    inline bool code39_t::check_bar_vector(const bar_vector_t& b,psize_t old_psize) const
    {
        // check psize
        // check colors
        assert(b.size() == 9);
#if 0
        return 
            (!old_psize || fabs((long)b.psize - (long)old_psize) < 0.5 * old_psize) && 
            b[0].first && b[8].first;
#else
        if (old_psize && ! (fabs((long) b.psize - (long) old_psize) < 0.5 * old_psize)) {
            return false;
        }
        if ( ! (b[0].first && b[8].first) ) {
                return false;
        }
        return true;
#endif
    }

    template<class TIT>
    bool code39_t::expect_n(TIT& start, TIT end, psize_t old_psize) const
    {
        using namespace scanner_utilities;
        bar_vector_t b(1);
        if ( get_bars(start,end,b,1) != 1 || b[0].first ) return false;
#ifdef STRICT
        u_t n_l = ((double) old_psize / 15.0); // ((b.size / (6*1+3*3)) * 1
        u_t n_h = ((double) old_psize / 12.0); // ((b.size / (6*1+3*2)) * 1
#else
        u_t n_l = ((double) old_psize / 30.0);
        u_t n_h = ((double) old_psize / 7.0);
#endif
        return n_l <= b[0].second && b[0].second <= n_h;
    }

    template<class TIT>
    scanner_result_t code39_t::scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match start marker
        // do relatively cheap though rough test on the first two bars only.
        bar_vector_t b(9);
        if ( get_bars(start,end,b,2) != 2 ) return scanner_result_t();
        if (b[0].second > 0.8 * b[1].second) return scanner_result_t();
        if (b[1].second > 3.5 * b[0].second) return scanner_result_t();

        if ( add_bars(start,end,b,7) != 7 ) return scanner_result_t();
        if (! check_bar_vector(b) ) return scanner_result_t();

        // check quiet_zone with respect to length of the first symbol
        //if (quiet_psize < (double) b.psize * 0.7) return scanner_result_t(); // 10 x quiet zone
        if (quiet_psize < (double) b.psize * 0.4) return scanner_result_t(); // 10 x quiet zone

        // expect start sequence
        module_word_t key = get_key(b);
        if ( ! key || table[key] != DELIMITER ) {
            return scanner_result_t();
        }

        std::string code = "";
        psize_t old_psize;
        bool at_end = false;
        while (! at_end) {
            old_psize = b.psize;
            // consume narrow separator
            if (! expect_n(start,end,old_psize)) return scanner_result_t();

            // get new symbol
            if ( get_bars(start,end,b,9) != 9) return scanner_result_t();
            if (! check_bar_vector(b,old_psize) ) return scanner_result_t();

            key = get_key(b);
            if (! key ) return scanner_result_t();
            const uint8_t c = table[key];
            switch(c) {
            case (uint8_t) no_entry: return scanner_result_t();
            case (uint8_t) DELIMITER: at_end = true; break;
            default: code.push_back(c);
            }
        }

        return scanner_result_t(code39,code,x,y);
    }

    template<class TIT>
    scanner_result_t code39_t::reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match start marker
        // do relatively cheap though rough test on the first two bars only.
        bar_vector_t b(9);
        if ( get_bars(start,end,b,2) != 2 ) return scanner_result_t();
        if (b[0].second > 1.8 * b[1].second) return scanner_result_t();
        if (b[1].second > 1.8 * b[0].second) return scanner_result_t();

        if ( add_bars(start,end,b,7) != 7 ) return scanner_result_t();
        if (! check_bar_vector(b) ) return scanner_result_t();

        // check quiet_zone with respect to length of the first symbol
        if (quiet_psize < (double) b.psize * 0.4) return scanner_result_t(); // 10 x quiet zone

        // expect start sequence
        module_word_t key = reverse_get_key(b);
        if ( ! key || table[key] != DELIMITER ) {
            return scanner_result_t();
        }

        std::string code = "";
        psize_t old_psize;
        bool at_end = false;
        while (! at_end) {
            old_psize = b.psize;
            // consume narrow separator
            if (! expect_n(start,end,old_psize)) return scanner_result_t();

            // get new symbol
            if ( get_bars(start,end,b,9) != 9) return scanner_result_t();
            if (! check_bar_vector(b,old_psize) ) return scanner_result_t();

            key = reverse_get_key(b);
            if (! key ) return scanner_result_t();
            const uint8_t c = table[key];
            switch(c) {
            case (uint8_t) no_entry: return scanner_result_t();
            case (uint8_t) DELIMITER: at_end = true; break;
            default: code.push_back(c);
            }
        }

        return scanner_result_t(code39,std::string(code.rbegin(),code.rend()),x,y);
    }

}; // namespace BarDecode

#endif // _CODE39_HH_
