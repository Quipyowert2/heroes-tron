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

#include "config.h"
#include <stdlib.h>
#include <stdio.h>

#include "pcx.h"
#include "display.h"
#include "keyboard_map.h"
#include "timer.h"
#include "fastmem.h"
#include "start.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

#define XBUF 128
#define YBUF 324
static char *scroll_buffer;
static char *page;
static unsigned int *jumps;
static char *colors;
static int nbrsauts;

static image_ background_img, texte;

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
  char *dest = page;
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
  fastmem4 (page, screen, 320 * 200 / 4);
}

static void
precalcsauts (void)
{
  int i;
  char cur;
  int size;
  char *src = texte.buffer;

  texte.buffer[63999] = 1;

  /* 1ère passe */
  cur = *src;
  i = 0;
  while (cur != 1) {
    size = 0;
    while (*src == cur) {
      size++;
      src++;
    }
    i++;
    cur = *src;
  }

  jumps = (unsigned int *) malloc (i * sizeof (unsigned int));
  colors = (char *) malloc (i);
  nbrsauts = i;

  /* 2ème passe */
  src = texte.buffer;
  cur = *src;
  i = 0;
  while (cur != 1) {
    size = 0;
    while (*src == cur) {
      size++;
      src++;
    }
    jumps[i] = size;
    colors[i] = cur;
    i++;
    cur = *src;
  }
}

static void
preptext (void)
{
  int i = 0, j;
  char *dest = page;
  char c;

  do {
    if (colors[i] == 0)
      dest += jumps[i];
    else {
      c = colors[i];
      for (j = jumps[i]; j != 0; j--)
	*dest++ = c;
    }
    i++;
  } while (i < nbrsauts);
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
draw_background (int pas)
{
  static int frame = 0;
  draw_background ((XBUF / 2) - 160 + ls (2 * frame / 3 /*+(LPI>>1) */ ),
		   (YBUF / 2) - 100 + ls ( /*3* */ frame /* /2 */ ));
  frame += (pas << 8);
  preptext ();
}

void
run1st (void)
{
  signed char p;
  int t, fr = 1;
  FILE *aux;
  char flag;

  aux = fopen (confdir "run.dat", "rb");
  if (aux) {
    fread ((char *) &flag, 1, 1, aux);
    fclose (aux);
    if (flag)
      return;
  }

  scroll_buffer = (char *) malloc (XBUF * YBUF);
  page = (char *) malloc (320 * 200);
  if (scroll_buffer == NULL || page == NULL)
    return;

  pcx_load (introdir "start.pcx", &background_img);
  pcx_load (introdir "starttxt.pcx", &texte);
  copy_background ();
  precalcsauts ();
  img_free (&background_img);	/* libere seulement le buffer, pas la palette... */

  modevga (G320x200x256);

  set_pal_fade (0);

  init_timer ();
  p = 0;
  do {
    draw_background (fr);
    vsynchro ();
    fr = read_and_reset_timer_non_zero ();
    if (p < 64) {
      p += fr;
      if (p > 64)
	p = 64;
      set_pal_fade (p);
    }
    display_page ();
  } while (key_ready () == 0);
  t = get_key ();
  if ((t >> 8) == 0x1f)
    flag = 1;
  do {
    draw_background (fr);
    vsynchro ();
    fr = read_and_reset_timer_non_zero ();
    if (p > 0) {
      p -= fr;
      if (p < 0)
	p = 0;
      set_pal_fade (p);
    }
    display_page ();
  } while (p > 0);
  uninit_timer ();

  modevga (TEXT);

  if (flag) {
    aux = fopen (confdir "run.dat", "rb+");
    if (aux) {
      fwrite ((char *) &flag, 1, 1, aux);
      fclose (aux);
    }
  }

  img_free (&texte);
  free (page);
  free (scroll_buffer);
  free (jumps);
  free (colors);
}
