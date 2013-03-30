/*
 * Gaussian Blur.
 * Copyright (C) 2008, Valentin Ziegler
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
 */

#ifndef GAUSSIAN_BLUR_HH
#define GAUSSIAN_BLUR_HH

#include "Image.hh"

// when radius <= 0, then an optimal radius for sd is used
void GaussianBlur(Image& image, double standard_deviation, int radius=0);

#endif
