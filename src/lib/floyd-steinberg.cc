/*
 * Floyd Steinberg dithering based on web publications.
 *
 * Copyright (C) 2006 - 2008 René Rebe, ExactCOD GmbH Germany
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
 *
 */

#include <stdlib.h>	// malloc / free
#include <math.h>	// floor
#include <inttypes.h>	// uint8_t

#include "floyd-steinberg.h"

void
FloydSteinberg (uint8_t* src_row, int width, int height, int shades, int bytes)
{
  int direction = 1;
  
  const float factor = (float) (shades - 1) / (float) 255;
  
  /*  allocate row/error buffers  */
  float* error = (float*) malloc (width * bytes * sizeof (float));
  float* nexterror = (float*) malloc (width * bytes * sizeof (float));
  
  /*  initialize the error buffers  */
  for (int row = 0; row < width * bytes; row++)
    error[row] = nexterror[row] = 0;

  for (int row = 0; row < height; row++)
    {
      int start, end;

      for (int col = 0; col < width * bytes; col++)
	nexterror[col] = 0;

      if (direction == 1) {
	  start = 0;
	  end = width;
      }
      else {
	direction = -1;
	start = width - 1;
	end = -1;
      }
      
      for (int col = start; col != end; col += direction)
	{
	  for (int channel = 0; channel < bytes; channel++)
	    {
     
	  float newval =
	    src_row[col * bytes + channel] + error[col * bytes + channel];

          newval = floor (newval*factor + 0.5) / factor;
	  if (newval > 255)
	    newval = 255;
	  else if (newval < 0)
	    newval = 0;

	  uint8_t newvali = (uint8_t)(newval+0.5);
	  
	  float cerror = src_row[col * bytes + channel] + error[col * bytes +
							  channel] - newvali;
	  src_row[col * bytes + channel] = newvali;
	  
	  // limit color bleeding, limit to /4 of the sample range
	  // TODO: make optional
	  if (fabs(cerror) > 255 / 4) {
	    if (cerror < 0)
	      cerror = -255 / 4;
	    else
	      cerror = 255 / 4;
	  }
	  
	  nexterror[col * bytes + channel] += cerror * 5 / 16;
	  if (col + direction >= 0 && col + direction < width)
	    {
	      error[(col + direction) * bytes + channel] += cerror * 7 / 16;
	      nexterror[(col + direction) * bytes + channel] +=
		cerror * 1 / 16;
	    }
	  if (col - direction >= 0 && col - direction < width)
	    nexterror[(col - direction) * bytes + channel] += cerror * 3 / 16;

	    }
	}

      src_row += width*bytes;

      /* next row in the opposite direction */
      direction *= -1;

      /* swap error/nexterror */
      float* tmp = error;
      error = nexterror;
      nexterror = tmp;
    }

  free (error);
  free (nexterror);
}
