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

#include "Scanner.hh"

namespace BarDecode
{
    std::ostream& operator<< (std::ostream& s, const code_t& t)
    {
        switch(t) {
        case ean8: return s << "ean8";
        case ean13: return s << "ean13";
        case upca: return s << "upca";
        case ean8|ean13|upca: return s << "ean";
        case upce: return s << "upce";
        case code128: return s << "code128";
        case gs1_128: return s << "GS1-128";
        case code39: return s << "code39";
        case code39_mod43: return s << "code39_mod43";
        case code39_ext: return s << "code39_ext";
        case code25i: return s << "code25i";
        default: return s << "unknown barcode type";
        }
    }

}; // namespace BarDecode
