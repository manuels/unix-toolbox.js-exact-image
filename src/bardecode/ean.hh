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

#ifndef _EAN_HH_
#define _EAN_HH_

#include "scanner_utils.hh"

namespace BarDecode
{

    struct ean_t
    {
        enum {
            normal_guard = 1,
            center_guard = 2,
            special_guard = 3,
            add_on_guard = 4,
            add_on_delineator = 5
        };

        static const usize_t min_quiet_usize = 5;
//        static const usize_t min_quiet_usize = 7;

        ean_t();
        
        template<class TIT>
        scanner_result_t scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t, directions_t dir = any_direction);

        DECLARE_TABLE(table,128);
        DECLARE_TABLE(ean13table,64);
        DECLARE_TABLE(auxtable,32);
    };

    inline ean_t::ean_t()
    {
        // EAN Tables (table A,B,C are put into onei array) (5.1.1.2.1)
        INIT_TABLE(table,128,0);

        // EAN Table A (parity odd)
        PUT_IN_TABLE(table,0x0D,'0');
        PUT_IN_TABLE(table,0x19,'1');
        PUT_IN_TABLE(table,0x13,'2');
        PUT_IN_TABLE(table,0x3D,'3');
        PUT_IN_TABLE(table,0x23,'4');
        PUT_IN_TABLE(table,0x31,'5');
        PUT_IN_TABLE(table,0x2F,'6');
        PUT_IN_TABLE(table,0x3B,'7');
        PUT_IN_TABLE(table,0x37,'8');
        PUT_IN_TABLE(table,0x0B,'9');

        // EAN Table B (parity even)
#define EANB(a) (0x40&(((~a)&127)<<6)) | \
        (0x20&(((~a)&127)<<4)) | \
        (0x10&(((~a)&127)<<2)) | \
        (0x01&(((~a)&127)>>6)) | \
        (0x02&(((~a)&127)>>4)) | \
        (0x04&(((~a)&127)>>2)) | \
        (0x08&((~a)&127))
        // mirror of EANC
        PUT_IN_TABLE(table,EANB(0x0D),'0');
        PUT_IN_TABLE(table,EANB(0x19),'1');
        PUT_IN_TABLE(table,EANB(0x13),'2');
        PUT_IN_TABLE(table,EANB(0x3D),'3');
        PUT_IN_TABLE(table,EANB(0x23),'4');
        PUT_IN_TABLE(table,EANB(0x31),'5');
        PUT_IN_TABLE(table,EANB(0x2F),'6');
        PUT_IN_TABLE(table,EANB(0x3B),'7');
        PUT_IN_TABLE(table,EANB(0x37),'8');
        PUT_IN_TABLE(table,EANB(0x0B),'9');

        // EAN Table C (parity even)
#define EANC(a) (~a)&127  // bit complement of A (7 bit)
        PUT_IN_TABLE(table,EANC(0x0D),'0');
        PUT_IN_TABLE(table,EANC(0x19),'1');
        PUT_IN_TABLE(table,EANC(0x13),'2');
        PUT_IN_TABLE(table,EANC(0x3D),'3');
        PUT_IN_TABLE(table,EANC(0x23),'4');
        PUT_IN_TABLE(table,EANC(0x31),'5');
        PUT_IN_TABLE(table,EANC(0x2F),'6');
        PUT_IN_TABLE(table,EANC(0x3B),'7');
        PUT_IN_TABLE(table,EANC(0x37),'8');
        PUT_IN_TABLE(table,EANC(0x0B),'9');

        // EAN Auxiliary Pattern Table (5.1.1.2.2)
        INIT_TABLE(auxtable,32,0);
        PUT_IN_TABLE(auxtable,0x05,normal_guard); // normal guard pattern, 3 modules
        PUT_IN_TABLE(auxtable,0x0A,center_guard); // center guard pattern, 5 modules
        PUT_IN_TABLE(auxtable,0x15,special_guard); // special guard pattern, 6 modules
        PUT_IN_TABLE(auxtable,0x0B,add_on_guard); // add-on guard pattern, 4 modules
        PUT_IN_TABLE(auxtable,0x01,add_on_delineator); // add-on delineator, 2 modules

        INIT_TABLE(ean13table,64,0);
        PUT_IN_TABLE(ean13table,0x3f,'0');
        PUT_IN_TABLE(ean13table,0x34,'1');
        PUT_IN_TABLE(ean13table,0x32,'2');
        PUT_IN_TABLE(ean13table,0x31,'3');
        PUT_IN_TABLE(ean13table,0x2c,'4');
        PUT_IN_TABLE(ean13table,0x26,'5');
        PUT_IN_TABLE(ean13table,0x23,'6');
        PUT_IN_TABLE(ean13table,0x2a,'7');
        PUT_IN_TABLE(ean13table,0x29,'8');
        PUT_IN_TABLE(ean13table,0x25,'9');

    }

    // scanner_result_t() indicates failure
    template<class TIT>
    scanner_result_t ean_t::scan(TIT& start, TIT end, pos_t x, pos_t y,psize_t quiet_psize, directions_t dir)
    {
        using namespace scanner_utilities;

        bool scan_reverted = dir & right_left;
        bool scan_normal = dir & left_right;

        // try to match start marker

        // try ean with 3 bars
        bar_vector_t b(3);
        if ( get_bars(start,end,b,2) != 2) return scanner_result_t();
        if ( b[0].second > 2 * b[1].second || b[0].second < 0.5 * b[1].second ) return scanner_result_t();
        if ( add_bars(start,end,b,1) != 1) return scanner_result_t();

        // get a first guess for u
        u_t u = (double) b.psize / 3.0; // 3 is the number of modules of the start sequence

        // check if u is within max_u imposed by quiet_zone
        if ( u > max_u<ean_t>(quiet_psize) ) return scanner_result_t();

        // expect start sequence
        module_word_t mw = get_module_word_adjust_u(b,u,3);
        char result = auxtable[mw];
        std::string code = "";
        if (result != normal_guard) return scanner_result_t();

        // Ok, we found an ean start sequence, let's try to read the code:

        uint bps = 4; // (bars per symbol) a symbol has 4 bars
        module_word_t parities = 0;
        uint symbols_count = 0;
        result = 0;
        b.resize(bps);
        bool reverted = false;
        do {
            // get symbol
            if ( get_bars(start,end,b,4) != 4 ) return scanner_result_t();

            // decide if to go for a symbol (7 modules) or the center_guard (4 modules)
            // FIXME FIXME FIXME Getting this right is crucial! Do not use a value that is to big
            if ( symbols_count == 6 ) break;
            //if ( symbols_count == 4 && fabs(((double)b.psize / 4.0) - u) <= (u * 0.25) ) break;
            if ( symbols_count == 4 && get_module_word(b,u,4) ) break;
           
            // lets assume 7 modules
            module_word_t mw = get_module_word_adjust_u(b,u,7);
            if ( ! mw ) return scanner_result_t();
            result = table[mw];
            if (! result) return scanner_result_t();
            if ( symbols_count == 0 ) {
                if (! get_parity(mw) ) {
                    if (! scan_reverted ) return scanner_result_t();
                    reverted = true;
                } else if ( ! scan_normal ) return scanner_result_t();
            }
            ++symbols_count;
            if (! reverted) {
                parities <<= 1;
                parities |= get_parity(mw);
            }
            code += result;
        } while ( true ); // only way to leave is a failure return or the center_guard break above

        // check if we found a symbol at all
        if ( ! symbols_count ) return scanner_result_t();

        // consume the next bar (it belongs to the center_guard, that we expect)
        if (add_bars(start,end,b,1) != 1) return scanner_result_t();

        // expect the center guard (with 5 modules)
        mw = get_module_word_adjust_u(b,u,5);
        if ( ! mw  || auxtable[mw] != center_guard) return scanner_result_t();

        // TODO check for special guard (we need to implement add_bar() method

        // Decode the second half of the barcode // TODO
        if (symbols_count != 6 && symbols_count != 4) return scanner_result_t();
        for (uint i = 0; i < symbols_count; ++i) {

            if ( get_bars(start,end,b,4) != 4 ) return scanner_result_t();
            module_word_t mw = get_module_word_adjust_u(b,u,7);
            if ( ! mw ) return scanner_result_t();
            if (reverted) {
                result = table[0x3f & ~mw];
            } else {
                result = table[mw];
            }
            if (! result) return scanner_result_t();
            code += result;
            if (reverted) {
                parities >>= 1;
                parities |= (! get_parity(0x3f & ~mw) << 5);
            }
        }

        // expect normal guard
        if ( get_bars(start,end,b,3) != 3) return scanner_result_t();
        mw = get_module_word_adjust_u(b,u,3);
        if ( ! mw ) return scanner_result_t();
        result = auxtable[mw];
        if (result == normal_guard) {

            // TODO check right quiet zone

        } else if (result == add_on_guard) {

            // TODO
            assert(false && "TODO");

        } else {
            return scanner_result_t();
        }

        if (reverted) {
            code = std::string(code.rbegin(),code.rend());
        }
        // get type and lookup bit 0 for ean13
        code_t type = ean;
        if (symbols_count == 6) {
            result = ean13table[parities];
            if (! result) return scanner_result_t();
            if ( result == '0' ) {
                type = upca;
            } else {
                code = std::string(1,result) + code;
                type = ean13;
            }
        } else {
            type = ean8;
        }

        // checksum test
        int check = code[code.size()-1] - 48;
        code.erase(code.size()-1);
        int sum = 0;
        int f = 3;
        for (int i = (int)code.size()-1; i >= 0; --i) {
            sum += (code[i]-48)*f;
            f = ((f==1) ? 3 : 1);
        }
        if (10-(sum % 10) != check) return scanner_result_t();

        // scan modules according to code_type
        return scanner_result_t(type,code,x,y);

    }

}; // namespace BarDecode

#endif // _EAN_HH_
