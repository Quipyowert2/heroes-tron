/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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

#include "common.h"
#include "pcx.h"
#include "display.h"
#include "const.h"
#include "keyb.h"
#include "timer.h"
#include "fastmem.h"
#include "endscroll.h"

#define XBUF 128
#define YBUF 324
static char *scroll_buffer;
static char *page;
static unsigned int *jumps;
static char *colors;
/* static int nbrsauts; */

static image_ background_img;

#define LPI (1<<14)
#define LS(x) ( (x) * (LPI-(x)) >> 20 )
static signed long int
ls (signed long int x)
{
  x &= (LPI << 1) - 1;
  return (((x & LPI) ? (-LS (x & (LPI - 1))) : LS (x)) * 7 >> 3);

/* ça se passe de commentaires, non ? :-) */
}

static void
copy_background (void)
{
  int i;
  char *dest = scroll_buffer;
  char *src = background_img.buffer;

  for (i = 108; i != 0; i--) {
    fastmem4 (src, dest, 128 / 4);
    fastmem4 (src, dest + 108 * XBUF, 128 / 4);
    fastmem4 (src, dest + 216 * XBUF, 128 / 4);
    dest += XBUF;
    src += 128;
  }
}

static void
draw_background (int x, int y)
{
  int i;
  char *dest = page + 320 * 10;
  char *src = scroll_buffer + x + y * XBUF;

  for (i = 200; i != 0; i--) {
    fastmem4 (src, dest, 320 / 4);
    dest += 320;
    src += XBUF;
  }

}

static void
display_page (void)
{
  fastmem4 (page + 320 * 10, screen, 320 * 200 / 4);
}

static void
set_pal_fade (char p)
{
  palette_ paldest;
  int i;

  for (i = 767; i >= 0; i--)
    paldest.global[i] = (unsigned char)
      ((background_img.palette.global[i] * p) >>6);
  set_pal ((char *) &paldest, 0, 768);
}

static void
render_background (int pas)
{
  static int frame = 0;
  draw_background ((XBUF / 2) - 160 + ls (2 * frame / 3 /*+(LPI>>1) */ ),
		   (YBUF / 2) - 100 + ls ( /*3* */ frame /* /2 */ ));
  frame += (pas << 8);
}

int fr = 1;
#define SDF
#define __HEROES__
#include "gfx_reader.h"

void
end_scroll (void)
{
  /* signed char p; */
  /* int t; */
  /* FILE* aux; */
  /* char flag; */

  scroll_buffer = malloc (XBUF * YBUF);
  page = malloc (320 * 220);
  if (scroll_buffer == NULL || page == NULL)
    return;

  pcx_load_from_rsc ("end-scroller-bg-img", &background_img);
  copy_background ();
  img_free (&background_img); /* only free the buffer, not the palette */

  set_pal_fade (64);

  p = 0;
  graphic_reader ();
  free (page);
  free (scroll_buffer);
  free (jumps);
  free (colors);
}
