/*
 * The ExactImage stable external API for use with SWIG.
 * Copyright (C) 2006 - 2009 René Rebe, ExactCODE GmbH
 * Copyright (C) 2006 René Rebe, Archivista
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

%module ExactImage

%include "typemaps.i"

%include "cstring.i"
%include "std_string.i"

# manually include it, otherwise SWIG will not source it
%include "config.h"

%{
#include "api.hh"
%}

/* Parse the header file to generate wrappers */
%include "api.hh"
