/*------------------------------------------------------------------------.
| Copyright 2000, 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                         |
| This file is part of Heroes.                                            |
|                                                                         |
| Heroes is free software; you can redistribute it and/or modify it under |
| the terms of the GNU General Public License as published by the Free    |
| Software Foundation; either version 2 of the License, or (at your       |
| option) any later version.                                              |
|                                                                         |
| Heroes is distributed in the hope that it will be useful, but WITHOUT   |
| ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   |
| FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   |
| for more details.                                                       |
|                                                                         |
| You should have received a copy of the GNU General Public License along |
| with this program; if not, write to the Free Software Foundation, Inc., |
| 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                   |
`------------------------------------------------------------------------*/

#include "system.h"
#include "scrtools.h"
#include "fader.h"
#include "timer.h"
#include "video.h"
#include "prefs.h"
#include "const.h"
#include "fastmem.h"
#include "argv.h"
#include "camera.h"

void
set_pal_with_luminance (const palette_t* palsrc)
{
  palette_t paldest;
  int i;
  float gamma_coef = (opt.luminance + 5) / 11.0;

  /* preform gamma correction */
  for (i = 767; i >= 0; i--) {
    float color = palsrc->global[i];
    color = pow (color / 63.0, gamma_coef) * 63.0 + 0.5;
    if (color > 63.0)
      color = 63.0;
    paldest.global[i] = (u8_t) color;
  }

  set_pal (paldest.global, 0, 768);
}

void
set_palette (const unsigned char *palette)
{
  cancel_fader ();
  set_pal (palette, 0, 768);
}

void
flush_display (const pixel_t *src)
{
  run_fader ();
  vsynchro (src);
  update_htimers ();
}

void
flush_display2 (const pixel_t *src1, const pixel_t *src2)
{
  run_fader ();
  if (swapside)
    vsynchro2 (src2, src1);
  else
    vsynchro2 (src1, src2);
  update_htimers ();
}

void
flush_display_moving (int x)
{
  pixel_t *src = corner[0];
  int *desti;
  int i, j;

  src += x << 2;
  for (i = 200; i > 0; i--, src += xbuf) {
    memmove (src + 160 + (x << 2), src + 160 - (x << 2), 160 - (x << 2));
    desti = ((int *) src) + 40 - x;
    for (j = x << 1; j != 0; j--)
      *desti++ = 0;
  }
  flush_display (corner[0] + (x << 2));
}

void
flush_display2_moving (int x)
{
  pixel_t *src1 = corner[swapside];
  const pixel_t *src2 = corner[1 - swapside];
  int *desti;
  int i, j;

  src1 += x << 2;
  for (i = 200; i > 0; i--, src1 += xbuf, src2 += xbuf) {
    desti = ((int *) src1) + 40 - x;
    for (j = x << 1; j != 0; j--)
      *desti++ = 0;
    fastmem4 (src2, src1 + 160 + (x << 2), 160 / 4 - x);
  }
  flush_display (corner[swapside] + (x << 2));
}

void
shade_scr_area (const pixel_t *src, pixel_t *dest)
{
  unsigned row;
  unsigned col;

  for (row = 200; row; --row) {
    for (col = 0; col < 320; ++col)
      dest[col] = glenz[1][src[col]];
    dest += xbuf;
    src += xbuf;
  }
}

void
copy_scr_area (const pixel_t *src, pixel_t *dest)
{
  unsigned row;
  for (row = 200; row; --row) {
    fastmem4 (src, dest, 320/4);
    src += xbuf;
    dest += xbuf;
  }
}

void
copy_image_to_scr_area (const pcx_image_t *src, pixel_t *dest)
{
  unsigned row;
  const pixel_t *s = src->buffer;
  for (row = 200; row; --row) {
    fastmem4 (s, dest, src->width/4);
    s += src->width;
    dest += xbuf;
  }
}

void
clear_scr_area (pixel_t *dest)
{
  unsigned row;
  for (row = 200; row; --row) {
    memset (dest, 0, 320);
    dest += xbuf;
  }
}
