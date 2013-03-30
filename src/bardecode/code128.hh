/*
 * Copyright (C) 2007 - 2008 Lars Kuhtz, ExactCODE GmbH Germany.
 * Copyright (C) 2009 René Rebe, ExactCODE GmbH Germany.
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

#ifndef _CODE128_HH_
#define _CODE128_HH_

#include <stdio.h>
#include <list>

#include "scanner_utils.hh"

namespace BarDecode
{
    struct code128_t
    {
        enum { FNC1, FNC2, FNC3, FNC4, SHIFT, CODEA, CODEB, CODEC, STARTA, STARTB, STARTC, END };
        static const char no_entry = 255;
        enum code_set_t { code_set_a, code_set_b, code_set_c };
     
        static const usize_t min_quiet_usize = 5; 
        static const usize_t min_quiet_usize_right = 5;

        code128_t();

        template<class TIT>
        scanner_result_t scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;
        
        template<class TIT>
        scanner_result_t reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;

        std::string decode128(code_set_t code_set, module_word_t mw) const; 
        code_set_t shift_code_set(code_set_t code_set) const;
        module_word_t get_key(module_word_t mw) const;

        bool is_end_key(module_word_t key) const;
        bool is_start_key(module_word_t key) const;
        bool is_no_entry(module_word_t key) const;
        scanner_result_t decode_key_list(const std::list<module_word_t>& list, pos_t x, pos_t y) const;

        DECLARE_TABLE(table,512);
        DECLARE_TABLE(aaux,10);
        DECLARE_TABLE(baux,10);
        DECLARE_TABLE(caux,10);

    };

    inline code128_t::code128_t()
    {
        // Code128 (255 indicates invalid module_word)
        // Based on 11 bits, where the first bit is 1 and the last bit is 0,
        // hence only 9 bit are used for lookup.
        INIT_TABLE(table,512,no_entry);
        PUT_IN_TABLE(table,0x166,0);
        PUT_IN_TABLE(table,0x136,1);
        PUT_IN_TABLE(table,0x133,2);
        PUT_IN_TABLE(table,0x4C,3);
        PUT_IN_TABLE(table,0x46,4);
        PUT_IN_TABLE(table,0x26,5);
        PUT_IN_TABLE(table,0x64,6);
        PUT_IN_TABLE(table,0x62,7);
        PUT_IN_TABLE(table,0x32,8);
        PUT_IN_TABLE(table,0x124,9);
        PUT_IN_TABLE(table,0x122,10);
        PUT_IN_TABLE(table,0x112,11);
        PUT_IN_TABLE(table,0xCE,12);
        PUT_IN_TABLE(table,0x6E,13);
        PUT_IN_TABLE(table,0x67,14);
        PUT_IN_TABLE(table,0xE6,15);
        PUT_IN_TABLE(table,0x76,16);
        PUT_IN_TABLE(table,0x73,17);
        PUT_IN_TABLE(table,0x139,18);
        PUT_IN_TABLE(table,0x12E,19);
        PUT_IN_TABLE(table,0x127,20);
        PUT_IN_TABLE(table,0x172,21);
        PUT_IN_TABLE(table,0x13A,22);
        PUT_IN_TABLE(table,0x1B7,23);
        PUT_IN_TABLE(table,0x1A6,24);
        PUT_IN_TABLE(table,0x196,25);
        PUT_IN_TABLE(table,0x193,26);
        PUT_IN_TABLE(table,0x1B2,27);
        PUT_IN_TABLE(table,0x19A,28);
        PUT_IN_TABLE(table,0x199,29);
        PUT_IN_TABLE(table,0x16C,30);
        PUT_IN_TABLE(table,0x163,31);
        PUT_IN_TABLE(table,0x11B,32);
        PUT_IN_TABLE(table,0x8C,33);
        PUT_IN_TABLE(table,0x2C,34);
        PUT_IN_TABLE(table,0x23,35);
        PUT_IN_TABLE(table,0xC4,36);
        PUT_IN_TABLE(table,0x34,37);
        PUT_IN_TABLE(table,0x31,38);
        PUT_IN_TABLE(table,0x144,39);
        PUT_IN_TABLE(table,0x114,40);
        PUT_IN_TABLE(table,0x111,41);
        PUT_IN_TABLE(table,0xDC,42);
        PUT_IN_TABLE(table,0xC7,43);
        PUT_IN_TABLE(table,0x37,44);
        PUT_IN_TABLE(table,0xEC,45);
        PUT_IN_TABLE(table,0xE3,46);
        PUT_IN_TABLE(table,0x3B,47);
        PUT_IN_TABLE(table,0x1BB,48);
        PUT_IN_TABLE(table,0x147,49);
        PUT_IN_TABLE(table,0x117,50);
        PUT_IN_TABLE(table,0x174,51);
        PUT_IN_TABLE(table,0x171,52);
        PUT_IN_TABLE(table,0x177,53);
        PUT_IN_TABLE(table,0x1AC,54);
        PUT_IN_TABLE(table,0x1A3,55);
        PUT_IN_TABLE(table,0x18B,56);
        PUT_IN_TABLE(table,0x1B4,57);
        PUT_IN_TABLE(table,0x1B1,58);
        PUT_IN_TABLE(table,0x18D,59);
        PUT_IN_TABLE(table,0x1BD,60);
        PUT_IN_TABLE(table,0x121,61);
        PUT_IN_TABLE(table,0x1C5,62);
        PUT_IN_TABLE(table,0x98,63);
        PUT_IN_TABLE(table,0x86,64);
        PUT_IN_TABLE(table,0x58,65);
        PUT_IN_TABLE(table,0x43,66);
        PUT_IN_TABLE(table,0x16,67);
        PUT_IN_TABLE(table,0x13,68);
        PUT_IN_TABLE(table,0xC8,69);
        PUT_IN_TABLE(table,0xC2,70);
        PUT_IN_TABLE(table,0x68,71);
        PUT_IN_TABLE(table,0x61,72);
        PUT_IN_TABLE(table,0x1A,73);
        PUT_IN_TABLE(table,0x19,74);
        PUT_IN_TABLE(table,0x109,75);
        PUT_IN_TABLE(table,0x128,76);
        PUT_IN_TABLE(table,0x1DD,77);
        PUT_IN_TABLE(table,0x10A,78);
        PUT_IN_TABLE(table,0x3D,79);
        PUT_IN_TABLE(table,0x9E,80);
        PUT_IN_TABLE(table,0x5E,81);
        PUT_IN_TABLE(table,0x4F,82);
        PUT_IN_TABLE(table,0xF2,83);
        PUT_IN_TABLE(table,0x7A,84);
        PUT_IN_TABLE(table,0x79,85);
        PUT_IN_TABLE(table,0x1D2,86);
        PUT_IN_TABLE(table,0x1CA,87);
        PUT_IN_TABLE(table,0x1C9,88);
        PUT_IN_TABLE(table,0x16F,89);
        PUT_IN_TABLE(table,0x17B,90);
        PUT_IN_TABLE(table,0x1DB,91);
        PUT_IN_TABLE(table,0xBC,92);
        PUT_IN_TABLE(table,0x8F,93);
        PUT_IN_TABLE(table,0x2F,94);
        PUT_IN_TABLE(table,0xF4,95);
        PUT_IN_TABLE(table,0xF1,96);
        PUT_IN_TABLE(table,0x1D4,97);
        PUT_IN_TABLE(table,0x1D1,98);
        PUT_IN_TABLE(table,0xEF,99);
        PUT_IN_TABLE(table,0xF7,100);
        PUT_IN_TABLE(table,0x1AF,101);
        PUT_IN_TABLE(table,0x1D7,102);
        PUT_IN_TABLE(table,0x142,103);
        PUT_IN_TABLE(table,0x148,104);
        PUT_IN_TABLE(table,0x14e,105);
        PUT_IN_TABLE(table,0x11d,106);

        // Range 96-105. Use offset -96
        INIT_TABLE(aaux,10,255);
        PUT_IN_TABLE(aaux,0,FNC3); // 96
        PUT_IN_TABLE(aaux,1,FNC2); // 97
        PUT_IN_TABLE(aaux,2,SHIFT); // 98
        PUT_IN_TABLE(aaux,3,CODEC); // 99
        PUT_IN_TABLE(aaux,4,CODEB); // 100
        PUT_IN_TABLE(aaux,5,FNC4); // 101
        PUT_IN_TABLE(aaux,6,FNC1); // 102
        PUT_IN_TABLE(aaux,7,STARTA); // 103
        PUT_IN_TABLE(aaux,8,STARTB); // 104
        PUT_IN_TABLE(aaux,9,STARTC); // 105

        INIT_TABLE(baux,10,255);
        PUT_IN_TABLE(baux,0,FNC3); // 96
        PUT_IN_TABLE(baux,1,FNC2); // 97
        PUT_IN_TABLE(baux,2,SHIFT); // 98
        PUT_IN_TABLE(baux,3,CODEC); // 99
        PUT_IN_TABLE(baux,4,FNC4); // 100
        PUT_IN_TABLE(baux,5,CODEA); // 101
        PUT_IN_TABLE(baux,6,FNC1); // 102
        PUT_IN_TABLE(baux,7,STARTA); // 103
        PUT_IN_TABLE(baux,8,STARTB); // 104
        PUT_IN_TABLE(baux,9,STARTC); // 105

        INIT_TABLE(caux,10,255);
        PUT_IN_TABLE(caux,0,no_entry); // 96
        PUT_IN_TABLE(caux,1,no_entry); // 97
        PUT_IN_TABLE(caux,2,no_entry); // 98
        PUT_IN_TABLE(caux,3,no_entry); // 99
        PUT_IN_TABLE(caux,4,CODEB); // 100
        PUT_IN_TABLE(caux,5,CODEA); // 101
        PUT_IN_TABLE(caux,6,FNC1); // 102
        PUT_IN_TABLE(caux,7,STARTA); // 103
        PUT_IN_TABLE(caux,8,STARTB); // 104
        PUT_IN_TABLE(caux,9,STARTC); // 105
    };

    inline bool code128_t::is_end_key(module_word_t key) const
    {
        return table[key] == 106;
    }

    inline bool code128_t::is_no_entry(module_word_t key) const
    {
        return table[key] == no_entry;
    }

    inline bool code128_t::is_start_key(module_word_t key) const
    {
        return table[key] >= 103 && table[key] <= 105;
    }


    // "" indicates no_entry
    inline std::string code128_t::decode128(code_set_t code_set, module_word_t key) const
    {
        int c = table[key];
        if (c == no_entry) return "";
        if (c == 106) return std::string(1,END);
        switch (code_set) {
        case code_set_c:
            if (c < 100) {
                char str[2];
                sprintf(str,"%02d",c);
                return std::string(str);
            } else {
                return std::string(1,caux[c-96]);
            }
            break;
        case code_set_b:
            if (c < 96) {
                return std::string(1,c+32);
            } else {
                return std::string(1,baux[c-96]);
            }
            break;
        case code_set_a:
            if (c < 64) {
                return std::string(1,c+32);
            } else if ( c < 96 ) {
                return std::string(1,c-64);
            } else {
                return std::string(1,aaux[c-96]);
            }
            break;
        default: assert(false); return "";
        }
    }

    inline module_word_t code128_t::get_key(module_word_t mw) const
    {
        // assume first bit is 1 and last bit is 0
        // use only the 9 bits inbetween for lookup
        if ( ! ((1<<10)&mw) || (mw&1)) {
            return 0;
        }
        mw &= ((1<<10)-1);
        mw >>= 1;
        
        return mw;
    }

    inline code128_t::code_set_t code128_t::shift_code_set(code_set_t code_set) const
    {
       switch (code_set) {
       case code_set_a: return code_set_b;
       case code_set_b: return code_set_a;
       default: return code_set;
       }
    }

    // FNC1 as first symbol indicates GS1-128
    //      in third or subsequent position: ascii 29.
    // TODO FNC2 (anywhere) concatenate barcode with next barcode
    // TODO FNC3 initialize or reprogram the barcode reader with the current code
    // TODO FNC4 switch to extended ascii (latin-1 as default)
    //      (quiet complicated usage refer to GS1 spec 5.3.3.4.2)
    template<class TIT>
    scanner_result_t code128_t::scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match start marker
        bar_vector_t b(6);
        if (get_bars(start,end,b,2) != 2 ) return scanner_result_t();

        if (b[0].second > 3 * b[1].second || b[0].second < 1.2 * b[1].second) return scanner_result_t();
        if ( add_bars(start,end,b,4) != 4) return scanner_result_t();

        // get a first guess for u
        u_t u = (double) b.psize / 11; // 11 is the number of modules of the start sequence

        // check if u is within max_u imposed by quiet_zone
        if ( u > max_u<code128_t>(quiet_psize) ) return scanner_result_t();

        // expect start sequence
        code_set_t cur_code_set;
        module_word_t key = get_key(get_module_word_adjust_u(b,u,11));

        std::string result = decode128(code_set_a,key);

        switch (result[0]) {
        case STARTA: cur_code_set = code_set_a; break;
        case STARTB: cur_code_set = code_set_b; break;
        case STARTC: cur_code_set = code_set_c; break;
        default: return scanner_result_t();
        }

        std::list<module_word_t> key_list;
        key_list.push_back(key);

        while (! is_end_key(key) ) { 
            // get symbol
            if ( get_bars(start,end,b,6) != 6 ) return scanner_result_t();
            key = get_key(get_module_word_adjust_u(b,u,11));
            if ( ! key || is_no_entry(key) ) return scanner_result_t();
            else key_list.push_back(key);
        }

        // remove end_key
        key_list.pop_back();

        if (key_list.size() <= 1) return scanner_result_t();

        // decode key_list (including check-summing)
        return decode_key_list(key_list,x,y);
    }

    // Backward algorithm:
    // 1. match end
    // loop:
    // 2. get key
    // 3. check if key does not decode to no_entry (independent of table type!)
    // 4. store key
    // 5. until: starta, startb, or startc is found
    // 6. decode keys in reverted order and compute checksum (using code from loop body above)
    // return
    template<class TIT>
    scanner_result_t code128_t::reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match end marker

        // expect a black bar of 2 modules
        bar_vector_t b(7);
        if ( get_bars(start,end,b,2) != 2) return scanner_result_t();
        if (b[0].second > 3 * b[1].second || b[0].second < 1.2 * b[1].second) return scanner_result_t();

        if ( add_bars(start,end,b,5) != 5) return scanner_result_t();

        // get a first guess for u
        u_t u = (double) b.psize / 13; // 13 is the number of modules of the end sequence

        // check if u is within max_u imposed by quiet_zone
        if ( u > max_u<code128_t>(quiet_psize) ) return scanner_result_t();

        // expect end sequence (well, the first 6 bars (or 7 modules) of it)
        b.erase(b.begin(),++b.begin());
        module_word_t key = get_key(reverse_get_module_word(b,u,11));
        if ( ! is_end_key(key)) return scanner_result_t();

        std::list<module_word_t> key_list;
        while (! is_start_key(key) ) { 
            // get symbol
            if ( get_bars(start,end,b,6) != 6 ) return scanner_result_t();
            key = get_key(reverse_get_module_word_adjust_u(b,u,11));
            if ( ! key || is_no_entry(key) ) return scanner_result_t();
            else key_list.push_front(key);
        } 

        if (key_list.size() <= 1) return scanner_result_t();

        // decode key_list (including check-summing)
        return decode_key_list(key_list,x,y);
    }

    inline scanner_result_t code128_t::decode_key_list(const std::list<module_word_t>& list, pos_t x, pos_t y) const
    {
        // initialize checksum:
        long checksum = table[list.front()];
        long pos = 0;
        std::string code = "";
        code_t type = code128;
        bool shift = false;
        code_set_t cur_code_set = code_set_a;

        for (std::list<module_word_t>::const_iterator it = list.begin(); it != --list.end(); ++it) {

            const module_word_t& key = *it;

            std::string result = decode128( (shift ? shift_code_set(cur_code_set) : cur_code_set), key);
            shift = false;
            switch (result.size()) {
            case 0: return scanner_result_t();
            case 2: code += result; break;
            case 1: 
                    switch (result[0]) {
                    case END: return scanner_result_t();
                    case SHIFT: shift = true; break;
                    case STARTA:
                    case CODEA: cur_code_set = code_set_a; break;
                    case STARTB:
                    case CODEB: cur_code_set = code_set_b; break;
                    case STARTC:
                    case CODEC: cur_code_set = code_set_c; break;
                    case FNC1: 
                                if (pos == 1) {
                                    type = gs1_128;
                                } else {
                                  code.push_back(29);
                                }
                                break;
                    case FNC2:
                    case FNC3:
                    case FNC4: std::cerr << "WARNING: Function charaters for code128 are not yet supported." << std::endl;
                                break;
                    default: code += result;
                    }
                    break;
            default: return scanner_result_t();
            }

            // first run will be 0, last run (check-digit is ommitted in the loop)
            checksum += pos * (long) table[key];
            ++pos;

        }

        // Checksum and return result
        if (  (checksum % 103) != (long) table[list.back()] ) {
            //std::cerr << "WARNING: checksum test for code128 failed on \""<< code << "\"." << std::endl;
            //std::cerr << "         checksum: " << checksum << " % 103 = " << checksum % 103 << " != " << (long) table[list.back()] << std::endl;
            return scanner_result_t();
        } else {
            return scanner_result_t(type,code,x,y);
        }
    }

}; // namespace BarDecode

#endif // _CODE128_HH_
