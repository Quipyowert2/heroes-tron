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
#include "display.h"
#include "options.h"
#include "const.h"
#include "fastmem.h"
#include "argv.h"

/* FIXME: perform gamma correction here */
void
set_pal_with_luminance (const palette_t* palsrc)
{
  palette_t paldest;
  int i;
  int lum = (3 - opt.luminance) * 6;

  if (opt.luminance == 3) {
    set_pal (palsrc->global, 0, 768);
    return;
  }
  if (opt.luminance < 3) {
    for (i = 767; i >= 0; i--)
      paldest.global[i] =
	(unsigned char) ((palsrc->global[i] * (64 - lum) + 63 * lum) >>6);
  } else if (opt.luminance > 3) {
    for (i = 767; i >= 0; i--)
      paldest.global[i] =
	(unsigned char) ((palsrc->global[i] * 64) / (64 - lum));
  }
  set_pal (paldest.global, 0, 768);
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

/* FIXME: remove */
void
vsynch (void)
{
  flush_display (screen);
}

void
backup_screen (pixel_t *dest)
{
  unsigned row;
  const pixel_t *src = screen;
  for (row = 200; row; --row) {
    fastmem4 (src, dest, 320/4);
    src += xbuf;
    dest += xbuf;
  }
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

void
aff_buffer (void)
{
  unsigned row;
  const pixel_t *src = corner[0];
  pixel_t *dest = screen;
  for (row = 200; row; --row) {
    fastmem4 (src, dest, 320/4);
    src += xbuf;
    dest += xbuf;
  }
}
