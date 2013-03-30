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

#ifndef _CODE25I_HH_
#define _CODE25I_HH_

#include "scanner_utils.hh"

namespace BarDecode
{
    struct code25i_t
    {
        code25i_t();

        static const int START_SEQUENCE = 0xA;
        static const int END_SEQUENCE = 0xD;
        static const char no_entry = 0;

        static const double n_lq = 15;
        static const double n_hq = 5.3;
        static const double w_lq = 5.2;
        static const double w_hq = 1.5;
        static const double tol = 0.2;

        static const usize_t min_quiet_usize = 5;
        //static const usize_t min_quiet_usize = 10;
        static const usize_t min_quiet_usize_right = 5;

        template<class TIT>
        scanner_result_t scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;

        template<class TIT>
        scanner_result_t reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t) const;

        bool check_bar_vector(const bar_vector_t& b,psize_t old_psize, double bdisprop) const;
        bool reverse_check_bar_vector(const bar_vector_t& b,psize_t old_psize, double bdisprop) const;

        std::pair<module_word_t,module_word_t> reverse_get_keys(const bar_vector_t& b) const;
        std::pair<module_word_t,module_word_t> get_keys(const bar_vector_t& b) const;

        DECLARE_TABLE(table,0x19);
    };

    inline code25i_t::code25i_t()
    {
        INIT_TABLE(table,0x18,no_entry);
        PUT_IN_TABLE(table,0x6,'0');
        PUT_IN_TABLE(table,0x11,'1');
        PUT_IN_TABLE(table,0x9,'2');
        PUT_IN_TABLE(table,0x18,'3');
        PUT_IN_TABLE(table,0x5,'4');
        PUT_IN_TABLE(table,0x14,'5');
        PUT_IN_TABLE(table,0xC,'6');
        PUT_IN_TABLE(table,0x3,'7');
        PUT_IN_TABLE(table,0x12,'8');
        PUT_IN_TABLE(table,0xA,'9');
    }


    inline std::pair<module_word_t,module_word_t> code25i_t::get_keys(const bar_vector_t& b) const
    {
        assert(b.size() == 10);

#ifdef STRICT
        u_t n_l = ((double) b.psize / 18.0); // (((b.size/2) / (3*1+2*3)) * 1
        u_t n_h = ((double) b.psize / 14.0); // (((b.size/2) / (3*1+2*2)) * 1
        u_t w_l = ((double) b.psize / 7.0);  // (((b.size/2) / (3*1+2*2)) * 2
        u_t w_h = ((double) b.psize / 6.0);   // (((b.size/2) / (3*1+2*3)) * 3
#else

        u_t bn_l = ((double) b.bpsize / n_lq);
        u_t bn_h = ((double) b.bpsize / n_hq);
        u_t bw_l = ((double) b.bpsize / w_lq);
        u_t bw_h = ((double) b.bpsize / w_hq);
        
        u_t wn_l = ((double) b.wpsize / n_lq);
        u_t wn_h = ((double) b.wpsize / n_hq);
        u_t ww_l = ((double) b.wpsize / w_lq);
        u_t ww_h = ((double) b.wpsize / w_hq);

#endif
        module_word_t r1 = 0;
        module_word_t r2 = 0;
        for (uint i = 0; i < 10; ++i) {
            r1 <<= 1;
            if (bw_l <= b[i].second && b[i].second <= bw_h) r1 += 1;
            else if (! (bn_l <= b[i].second && b[i].second <= bn_h)) {
                return std::make_pair(0,0);
            }

            ++i;
            r2 <<= 1;
            if (ww_l <= b[i].second && b[i].second <= ww_h) r2 += 1;
            else if (! (wn_l <= b[i].second && b[i].second <= wn_h)) {
                return std::make_pair(0,0);
            }
        }
        return std::make_pair(r1,r2);
    }

    inline std::pair<module_word_t,module_word_t> code25i_t::reverse_get_keys(const bar_vector_t& b) const
    {
        assert(b.size() == 10);

#ifdef STRICT
        u_t n_l = ((double) b.psize / 18.0); // (((b.size/2) / (3*1+2*3)) * 1
        u_t n_h = ((double) b.psize / 14.0); // (((b.size/2) / (3*1+2*2)) * 1
        u_t w_l = ((double) b.psize / 7.0);  // (((b.size/2) / (3*1+2*2)) * 2
        u_t w_h = ((double) b.psize / 6.0);   // (((b.size/2) / (3*1+2*3)) * 3
#else

        u_t bn_l = ((double) b.bpsize / n_lq);
        u_t bn_h = ((double) b.bpsize / n_hq);
        u_t bw_l = ((double) b.bpsize / w_lq);
        u_t bw_h = ((double) b.bpsize / w_hq);
        
        u_t wn_l = ((double) b.wpsize / n_lq);
        u_t wn_h = ((double) b.wpsize / n_hq);
        u_t ww_l = ((double) b.wpsize / w_lq);
        u_t ww_h = ((double) b.wpsize / w_hq);

#endif
        module_word_t r1 = 0;
        module_word_t r2 = 0;
        for (int i = 9; i >= 0; --i) {
            r1 <<= 1;
            if (bw_l <= b[i].second && b[i].second <= bw_h) r1 += 1;
            else if (! (bn_l <= b[i].second && b[i].second <= bn_h)) {
                return std::make_pair(0,0);
            }

            --i;
            r2 <<= 1;
            if (ww_l <= b[i].second && b[i].second <= ww_h) r2 += 1;
            else if (! (wn_l <= b[i].second && b[i].second <= wn_h)) {
                return std::make_pair(0,0);
            }
        }
        return std::make_pair(r2,r1);
    }

    inline bool code25i_t::check_bar_vector(const bar_vector_t& b,psize_t old_psize, double bdisprop) const
    {
        // check psize
        // check colors
        assert(b.size() == 10);
#if 0
        return 
            (!old_psize || fabs((long)b.psize - (long)old_psize) < 0.5 * old_psize) && 
            b[0].first && b[8].first;
#else
        if (old_psize && ! (fabs((long) b.psize - (long) old_psize) < 0.5 * old_psize)) {
            return false;
        } else
        if (b.bpsize < (1-tol) * (b.psize * bdisprop * 0.5) || 
            b.bpsize > (1+tol) * (b.psize * bdisprop * 0.5)) {
            return false;
        } else
        if ( ! b[0].first || b[9].first ) {
            return false;
        } else
            return true;
#endif
    }

    inline bool code25i_t::reverse_check_bar_vector(const bar_vector_t& b,psize_t old_psize, double bdisprop) const
    {
        // check psize
        // check colors
        assert(b.size() == 10);
#if 0
        return 
            (!old_psize || fabs((long)b.psize - (long)old_psize) < 0.5 * old_psize) && 
            b[0].first && b[8].first;
#else
        if (old_psize && ! (fabs((long) b.psize - (long) old_psize) < 0.5 * old_psize)) {
            return false;
        } else
        if (b.bpsize < (1-tol) * (b.psize * bdisprop * 0.5) || 
            b.bpsize > (1+tol) * (b.psize * bdisprop * 0.5)) { 
            return false;
        } else
        if ( b[0].first || ! b[9].first ) {
            return false;
        } else
            return true;
#endif
    }

    template<class TIT>
    scanner_result_t code25i_t::scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match start marker
        // do relatively cheap though rough test on the first two bars only.
        bar_vector_t b(4);
        if ( get_bars(start,end,b,2) != 2 ) return scanner_result_t();
        // lax-check (assymetric: we assume that black bars might be wider)
        if (b[0].second < 0.7 * b[1].second || b[0].second > 3 * b[1].second) return scanner_result_t();
        
        // check quiet_zone with respect to length of the first symbol
        if (quiet_psize < (double) (b[0].second + b[1].second) * 5 * 0.5) return scanner_result_t(); // 10 x quiet zone

        if ( add_bars(start,end,b,2) != 2 ) return scanner_result_t();
        // strict-check
        if (b[0].second < 0.7 * b[2].second || b[0].second > 1.3 * b[2].second) return scanner_result_t();
        if (b[1].second < 0.7 * b[3].second || b[1].second > 1.3 * b[3].second) return scanner_result_t();

        const double wdisprop = b.wpsize / (b.psize * 0.5);
        const double bdisprop = b.bpsize / (b.psize * 0.5);

        std::string code;
        psize_t old_psize = 0;
        bool at_end = false;
        while (! at_end) {

            // get new symbols
            if ( get_bars(start,end,b,3) != 3) return scanner_result_t();

            // check END sequence and expect quiet zone
            // TODO compare b[1] with value from start sequence? or better use disprop factor.
            if ( b[0].second < b[2].second * 3.1 * 1.3 &&
                 b[0].second > b[2].second * 2 * 0.7 &&
                 b[1].second < b.psize * 0.25 * (1+tol) * wdisprop  &&
                 b[1].second > b.psize * 0.18 * (1-tol) * wdisprop) {
                if ((start+1)->second > b.psize * 1.3) {
                    break;
                }
            }

            if ( add_bars(start,end,b,7) != 7) return scanner_result_t();
            if (! check_bar_vector(b,old_psize,bdisprop) ) return scanner_result_t();
            old_psize = b.psize;

            std::pair<module_word_t,module_word_t> keys = get_keys(b);
            if (! keys.first || ! keys.second ) return scanner_result_t();

            const char c1 = table[keys.first];
            if (c1 == no_entry) return scanner_result_t();
            code.push_back(c1);

            const char c2 = table[keys.second];
            if (c2 == no_entry) return scanner_result_t();
            code.push_back(c2);
        }
        if ( code.empty() ) return scanner_result_t();
        else {
            return scanner_result_t(code25i,code,x,y);
        }
    }

    template<class TIT>
    scanner_result_t code25i_t::reverse_scan(TIT& start, TIT end, pos_t x, pos_t y, psize_t quiet_psize) const
    {
        using namespace scanner_utilities;

        // try to match end marker: 1 1 2 / 1 1 3
        bar_vector_t b(3);
        if ( get_bars(start,end,b,2) != 2 ) return scanner_result_t();
        // lax-check (assymetric)
        if (b[0].second < 0.7 * b[1].second || b[0].second > 3 * b[1].second) return scanner_result_t();

        // check quiet_zone with respect to length of the first symbol
        if (quiet_psize < (double) (b[0].second + b[1].second) * 5 * 0.5) return scanner_result_t(); // 10 x quiet zone

        if ( add_bars(start,end,b,1) != 1 ) return scanner_result_t();
        // strict-check (TODO)
        if ( b[0].second < 0.3 * 0.7 * b[2].second || b[0].second > 0.5 * 1.3 * b[2].second ) return scanner_result_t();

        const double bdisprop = b.bpsize / (b.psize * 0.75);

        std::string code = "";
        psize_t old_psize = 0;
        bool at_end = false;
        while (! at_end) {

            // get new symbols
            if ( get_bars(start,end,b,4) != 4) return scanner_result_t();

            // check START sequence and expect quiet zone
            static const double ltol = 0.3;
            if ( b.bpsize > (1-tol) * (b.psize * bdisprop * 0.5) &&
                 b.bpsize < (1+tol) * (b.psize * bdisprop * 0.5) &&
                 (b.bpsize / (double) b[3].second) > 2 * (1-ltol) &&
                 (b.bpsize / (double) b[3].second) < 2 * (1+ltol) &&
                 (b.wpsize / (double) b[2].second) > 2 * (1-ltol) &&
                 (b.wpsize / (double) b[2].second) < 2 * (1+ltol) &&
                 (b.bpsize / (double) b[1].second) > 2 * (1-ltol) &&
                 (b.bpsize / (double) b[1].second) < 2 * (1+ltol) &&
                 (b.wpsize / (double) b[0].second) > 2 * (1-ltol) &&
                 (b.wpsize / (double) b[0].second) < 2 * (1+ltol)) {
                // FIXME make this in a more performant way
                if ((start+1)->second > b.psize * 1) {
                    break;
                }
            }

            if ( add_bars(start,end,b,6) != 6 ) return scanner_result_t();
            if (! reverse_check_bar_vector(b,old_psize,bdisprop) ) return scanner_result_t();
            old_psize = b.psize;

            std::pair<module_word_t,module_word_t> keys = reverse_get_keys(b);
            if (! keys.first || ! keys.second ) return scanner_result_t();

            const char c1 = table[keys.first];
            if (c1 == no_entry) return scanner_result_t();
            code.push_back(c1);

            const char c2 = table[keys.second];
            if (c2 == no_entry) return scanner_result_t();
            code.push_back(c2);
        }
        if ( code.empty() ) return scanner_result_t();
        else {
            return scanner_result_t(code25i,std::string(code.rbegin(),code.rend()),x,y);
        }
    }

}; // namespace BarDecode

#endif // _CODE25I_HH_
