/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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
#include "const.h"
#include "fastmem.h"
#include "options.h"
#include "display.h"
#include "draw.h"
#include "sprprogwav.h"

void
draw_text_bonus (const char* text, int posx, int posy, int p)
{
  char j, c;
  int i, k, l, m;
  unsigned char *dest = corner[p] + posx + posy * xbuf;
  const unsigned char *src = text;
  unsigned char sinl = text_waving_step;

  for (; *text != 0; text++) {
    i = (*text - 32);
    src =
      bonus_font_img.buffer + ((int) (i) % 26 * 12) +
      ((int) (i) / 26) * 320 * 12;
    for (j = 6; j != 0; j--) {
      sinl++;
      sinl &= 31;
      m = ((signed char) minisinus[sinl]) * xbuf;
      for (k = 320 * 10, l = xbuf * 10; k >= 0;) {
	c = *(src + k);
	k -= 320 * 2;
	if (c != 0) {
	  *(dest + m + l) = c;
	  *(dest + m + l + xbuf) = 82;
	}
	l -= xbuf * 2;
      }
      dest += 2;
      src += 2;
    }
  }
}

void
copy_rect_transp (const unsigned char *src, unsigned char *dest, int xt,
		  int yt)
{
  int j, k;
  for (j = yt; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      if (*src != 0)
	*dest = *src;
      src++;
      dest++;
    }
    src += 320 - xt;
    dest += xbuf - xt;
  }
}

void
copy_rect_transp_8 (const unsigned char *src, unsigned char *dest, int xt,
		    int yt, char coul)
{
  int j, k;
  char c;

  for (j = yt; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      c = *src++;
      if (c != 0) {
	if (c == 8)
	  *dest = coul;
	else
	  *dest = c;
      }
      dest++;
    }
    src += 320 - xt;
    dest += xbuf - xt;
  }
}

void
copy_rect_transp_red (const unsigned char *src, unsigned char *dest, int xt,
		      int yt)
{
  int j, k;

  if (opt.use_glenz)
    for (j = yt; j != 0; j--) {
      for (k = xt; k != 0; k--) {
	if (*src != 0)
	  *dest = glenz[6][*dest];
	src++;
	dest++;
      }
      src += 320 - xt;
      dest += xbuf - xt;
  } else
    for (j = yt; j != 0; j--) {
      for (k = xt; k != 0; k--) {
	if (*src != 0)
	  *dest = NOGLENZRED;
	src++;
	dest++;
      }
      src += 320 - xt;
      dest += xbuf - xt;
    }
}

void
copy_rect_4 (const unsigned char *src, unsigned char *dest, int xt, int yt)
{
  int j;
  for (j = yt; j > 0; j--) {
    fastmem4 (src, dest, xt >> 2);
    src += 320;
    dest += xbuf;
  }
}

void
copy_rect_2 (const unsigned char *src, unsigned char *dest, int xt, int yt)
{
  int j;
  for (j = yt; j > 0; j--) {
    fastmem2 (src, dest, xt >> 1);
    src += 320;
    dest += xbuf;
  }
}

void
draw_demo_stick (const pixel_t* dest ATTRIBUTE_UNUSED)
{
#ifdef PORT
  signed char sinl;
  /* FIXME: may be useful */
  if (demoversion) {
    sinl = (signed char) minisinus[(frame_old + 2) & 31];
    copy_rect_transp_8 (main_font_img.buffer + 66 + 91 * 320,
			dest + 185 * xbuf + 280 - /*sinl* */ (xbuf + 1), 37,
			12,
			10 - sinl);
  }
#endif
}

void
aff_buffer (void)
{
  unsigned char *src = corner[0];
  unsigned char *dest = (char *) screen;
  int i;
  draw_demo_stick (src);
  for (i = 200; i > 0; i--, src += xbuf, dest += 320)
    fastmem4 (src, dest, 320 / 4);
}
