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

#include "scanner_utils.hh"

#include "code128.hh"
#include "code25i.hh"
#include "code39.hh"
#include "ean.hh"

namespace BarDecode
{

    namespace
    {
        ean_t ean_impl;
        code128_t code128_impl;
        code39_t code39_impl;
        code25i_t code25i_impl;
    };

    // TODO make all tables static (would be nice to have something like designated initializers
    // like in C99 in C++ as well...) We do not have. Hence we need to work around.
    // possibly by encapsulating the tables into objects with constructors that
    // perform the initialization.
    template<bool v>
    void BarcodeIterator<v>::next()
    {
        assert( ! end());

        pos_t& vx = v ? y : x;
        pos_t& vy = v ? x : y;

        while (! end() ) {

            if (ti == te) {
                vx = 0;
                vy = v ? tokenizer.get_x() : tokenizer.get_y();
                tokenizer.next_line(token_line);
                ti = token_line.begin();
                te = token_line.end();
            }

            // goto next white space of psize >= min_quiet_psize, that is followed by black

            if ( ti+1 == te ) { ++ti; continue; }
            token_t t = *ti;
            token_t lookahead = *(ti+1);

            while ((! lookahead.first || t.first || t.second < min_quiet_psize) && (++ti + 1 != te) ) { // while black ...
                vx += t.second;
                t = lookahead;
                lookahead = *(ti+1);
            }
            if ( ti+1 == te ) { ++ti; continue; }
            
            assert(! t.first); // assert white

            /* ***************************** */
            // preliminary checks

            // each (non empty) Barcode has at least 24 modules (including both quiet zones)!
            if (te - ti < 24) { ti = te; continue; }

            psize_t quiet_psize = t.second;

            // check quiet_zone against minimal requirement from all code types
            if (lookahead.second * 3 > quiet_psize) { vx += t.second; ++ti; continue;}

            // not enough space left on line for minimal barcode width:
            //if (lookahead.second/3.0 * 14 + tokenizer.get_x() >= tokenizer.get_img()->w) continue;

            /* ***************************** */

            token_line_t::const_iterator backup_i = ti;
            scanner_result_t result;
            // try scanning for all requested barcode types
            if (directions&left_right && requested(code39)) {
                if ((result = code39_impl.scan(ti,te,x+ti->second,y,quiet_psize))) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            }
            if ( directions&right_left && requested(code39)) {
                ti =  backup_i;
                if ((result = code39_impl.reverse_scan(ti,te,x+ti->second,y,quiet_psize))) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            }
            if ( directions&left_right && requested(code25i)) {
                ti =  backup_i;
                if ((result = code25i_impl.scan(ti,te,x+ti->second,y,quiet_psize))) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            }
            if ( directions&right_left && requested(code25i)) {
                ti =  backup_i;
                if ((result = code25i_impl.reverse_scan(ti,te,x+ti->second,y,quiet_psize))) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            }
            if ( directions&left_right && requested(code128)) {
                ti =  backup_i;
                if (result = code128_impl.scan(ti,te,x+ti->second,y,quiet_psize)) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            } 
            if ( directions&right_left && requested(code128)) {
                ti =  backup_i;
                if (result = code128_impl.reverse_scan(ti,te,x+ti->second,y,quiet_psize)) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            } 
            if ( directions&(left_right|right_left) && requested(ean) ) {
                ti =  backup_i;
                if ((result = ean_impl.scan(ti,te,x+ti->second,y,quiet_psize,directions)) ) {
                    cur_barcode = result;
                    vx += pixel_diff(backup_i,ti);
                    return;
                }
            }
            vx += backup_i->second;
            ti = ++backup_i;
        }
    }    
}; // namespace BarDecode
