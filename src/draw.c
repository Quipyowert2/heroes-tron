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
#include "const.h"
#include "timer.h"
#include "fastmem.h"
#include "font_menu.h"
#include "font_deck.h"
#include "options.h"
#include "display.h"
#include "draw.h"

/* This really needs a rewrite */
#define JUSTIF_CALC(FCT)						\
  if (cent==0)                               /* flushed left  */	\
	d=0;								\
  else {								\
	  for(;*src!=0;src++) {						\
		i=(char)(FCT(*src)-font_first_ascii);			\
		d+=font_width[i];					\
	  }								\
	  if (cent==1)                       /* centered      */	\
		d=-(d>>1);						\
	  else                               /* flushed right */	\
		d=-d;							\
  }									\
  dest+=d;


void
draw_text (const unsigned char *texte, int posx, int posy, char cent)
{
  char j, c;
  int i, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * xbuf;
  const unsigned char *src = texte;

  JUSTIF_CALC (/**/);
  for (; *texte != 0; texte++) {
    i = (*texte - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    for (j = font_width[i]; j != 0; j--) {
      for (k = 320 * (font_height - 1), l = xbuf * (font_height - 1); 
	   k >= 0;) {
	c = *(src + k);
	k -= 320;
	if (c != font_transp_color)
	  *(dest + l) = c;
	l -= xbuf;
      }
      dest++;
      src++;
    }
  }
}

void
draw_text_clipped_left (const unsigned char *texte, int posx, int posy,
			char cent)
{
  char j, c;
  int i, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * xbuf;
  int clip = (int) (corner[0] + 0 + posy * xbuf);
  const unsigned char *src = texte;

  JUSTIF_CALC (/**/);
  for (; *texte != 0; texte++) {
    i = (*texte - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    for (j = font_width[i]; j != 0; j--) {
      if (((int) dest) >= clip)
	for (k = 320 * (font_height - 1), l = xbuf * (font_height - 1);
	     k >= 0;) {
	  c = *(src + k);
	  k -= 320;
	  if (c != font_transp_color)
	    *(dest + l) = c;
	  l -= xbuf;
	}
      dest++;
      src++;
    }
  }
}

void
draw_text_clipped_right (const unsigned char *texte, int posx, int posy,
			 char cent)
{
  char j, c;
  int i, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * xbuf;
  int clip = (int) (corner[0] + 320 + posy * xbuf);
  const unsigned char *src = texte;

  JUSTIF_CALC (/**/);
  for (; *texte != 0; texte++) {
    i = (*texte - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    for (j = font_width[i]; j != 0; j--) {
      if (((int) dest) < clip)
	for (k = 320 * (font_height - 1), l = xbuf * (font_height - 1);
	     k >= 0;) {
	  c = *(src + k);
	  k -= 320;
	  if (c != font_transp_color)
	    *(dest + l) = c;
	  l -= xbuf;
	}
      dest++;
      src++;
    }
  }
}

unsigned char text_waving_step = 0;
htimer_t text_waving_htimer = 0;

void
init_text_waving_step (void)
{
  text_waving_htimer = new_htimer (T_LOCAL, HZ (70));
  text_waving_step = 0;
}

void
uninit_text_waving_step (void)
{
  free_htimer (text_waving_htimer);
}

void
update_text_waving_step (void)
{
  text_waving_step += read_htimer (text_waving_htimer);
}

void
draw_text_waving (const unsigned char *texte, int posx, int posy, char cent)
{
  char j, c;
  int i, m, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * xbuf;
  const unsigned char *src = texte;
  unsigned char sinl = text_waving_step;

  JUSTIF_CALC (/**/);
  for (; *texte != 0; texte++) {
    i = (*texte - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    sinl += 2;
    sinl &= 31;
    m = ((signed char) minisinus[sinl]) * xbuf;
    for (j = font_width[i]; j != 0; j--) {
      for (k = 320 * (font_height - 1), l = xbuf * (font_height - 1) + m;
	   k >= 0;) {
	c = *(src + k);
	k -= 320;
	if (c != font_transp_color)
	  *(dest + l) = c;
	l -= xbuf;
      }
      dest++;
      src++;
    }
  }
}

void
draw_text_320 (const unsigned char *texte, int posx, int posy, char cent)
{
  char j, c;
  int i, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * 320;
  const unsigned char *src = texte;

  JUSTIF_CALC (toupper);
  for (; *texte != 0; texte++) {
    i = (toupper (*texte) - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    for (j = font_width[i]; j != 0; j--) {
      for (k = 320 * (font_height - 1), l = 320 * (font_height - 1); k >= 0;) {
	c = *(src + k);
	k -= 320;
	if (c != font_transp_color)
	  *(dest + l) = c;
	l -= 320;
      }
      dest++;
      src++;
    }
  }
}

void
draw_text_waving_320 (const unsigned char *texte, int posx, int posy, char cent)
{
  char j, c;
  int i, m, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * 320;
  const unsigned char *src = texte;
  unsigned char sinl = text_waving_step;

  JUSTIF_CALC (toupper);
  for (; *texte != 0; texte++) {
    i = (toupper (*texte) - font_first_ascii);
    src =
      main_font_img.buffer + font_pos + ((int) (i) % 14 * 22) +
      ((int) (i) / 14) * 320 * font_height;
    sinl += 2;
    sinl &= 31;
    m = ((signed char) minisinus[sinl]) * 320;
    for (j = font_width[i]; j != 0; j--) {
      for (k = 320 * (font_height - 1), l = 320 * (font_height - 1) + m;
	   k >= 0;) {
	c = *(src + k);
	k -= 320;
	if (c != font_transp_color)
	  *(dest + l) = c;
	l -= 320;
      }
      dest++;
      src++;
    }
  }
}

void (*draw_text_array[2]) (const unsigned char *, int, int, char) = {
  &draw_text, 
  &draw_text_waving
};

void (*draw_text_array_320[2]) (const unsigned char *, int, int, char) = {
  &draw_text_320, 
  &draw_text_waving_320
};

void
draw_text_bonus (const unsigned char *texte, int posx, int posy, int p)
{
  char j, c;
  int i, k, l, m;
  unsigned char *dest = corner[p] + posx + posy * xbuf;
  const unsigned char *src = texte;
  unsigned char sinl = text_waving_step;

  for (; *texte != 0; texte++) {
    i = (*texte - 32);
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

static int
deck_text_conv (char i)
{
  if (i >= 'a' && i <= 'z')
    i -= 'a' - 'A' + ' ';
  else if (i < ' ' || i > 'Z') 
    i = '*' - ' ';
  else
    i -= ' ';
  return i;
}

void
draw_deck_text (const unsigned char *texte, int posx, int posy, char cent)
{
  char c;
  int i, j, k, l, d = -1;
  unsigned char *dest = corner[0] + posx + posy * xbuf;
  const unsigned char *src = texte;

  if (cent == 0)		/* flushed left  */
    d = 0;
  else {
    for(; *src != 0; src++)
      d += font_deck_width[deck_text_conv (*src)] + 1;
    if (cent == 1)		/* centered      */
      d = -(d>>1);
    else			/* flushed right */
      d = -d;
  }
  dest += d;

  for (; *texte != 0; texte++) {
    i = deck_text_conv (*texte);

    src =  font_deck_img.buffer + ((int) (i) % 32 * 8) +
      ((int) (i) / 32) * 8 * 320 + 2 * 320;
    for (j = font_deck_width[i]; j != 0; --j) {
      for (k = 320 * (5 - 1), l = xbuf * (5 - 1); k >= 0;) {
	c = *(src + k);
	k -= 320;
	if (c)
	  *(dest + l) = c;
	l -= xbuf;
      }
      dest++;
      src++;
    }
    dest++;			/* Move one row (spacing between chars) */
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
copy_rect_transp_320 (const unsigned char *src, unsigned char *dest, int xt,
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
    dest += 320 - xt;
  }
}

void
copy_32x32_transp_z (const unsigned char *src, unsigned char *dest)
{
  int j, k;
  char c, d;
  for (j = 32; j != 0; j--) {
    for (k = 32; k != 0; k--) {
      c = *src++;
      if (c != 0) {
	d = *dest;
	if (d > 15 || d < c)
	  *dest = c;
      }
      dest++;
    }
    src += 320 - 32;
    dest += xbuf - 32;
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
copy_rect_2_320 (const unsigned char *src, unsigned char *dest, int xt,
		 int yt)
{
  int j;
  for (j = yt; j > 0; j--) {
    fastmem2 (src, dest, xt >> 1);
    src += 320;
    dest += 320;
  }
}

void
copy_rect_4_320 (const unsigned char *src, unsigned char *dest, int xt,
		 int yt)
{
  int j;
  for (j = yt; j > 0; j--) {
    fastmem4 (src, dest, xt >> 2);
    src += 320;
    dest += 320;
  }
}

void
draw_demo_stick (unsigned char *dest __attribute__ ((unused)))
{
#ifdef PORT
  signed char sinl;
  /* FIXME: may be usefull */
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

void
copy_rect_transp_shadow (const unsigned char *src, unsigned char *dest,
			 int xt, int yt)
{
  int j, k;
  for (j = yt; j != 0; j--) {
    for (k = xt; k != 0; k--) {
      if (*src != 0) {
	if (*src == 1)
	  *dest = glenz[0][*dest];
	else
	  *dest = *src;
      }
      src++;
      dest++;
    }
    src += 320 - xt;
    dest += xbuf - xt;
  }
}
