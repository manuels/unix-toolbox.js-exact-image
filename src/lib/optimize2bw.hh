/*
 * Copyright (C) 2005 - 2009 René Rebe
 *           (C) 2005 - 2007 Archivista GmbH, CH-8042 Zuerich
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

#ifndef OPTIMIZE2BW_HH
#define OPTIMIZE2BW_HH

#include "Image.hh"

// Optimizes the image for b/w images.
// It does not do the thresholding, the result is still 8 bit per pixel
// so the caller can scale on the shaded data.
// Threshold is not used, it just is a hint whether to use the more complex
// color code-path.

void optimize2bw (Image& image, int low = 0, int high = 0, int threshold = 0,
		  int sloppy_threshold = 0,
		  int radius = 3,
		  double standard_deviation = 2.1);

#endif // OPTIMIZE2BW_HH
