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

/* l'intro du jeu */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#include "display.h"
#include "pcx.h"
#include "timer.h"
#include "fastmem.h"

#include "const.h"
#include "sound.h"

#include "intro.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

/******* datas de l'intro *******/
palette_ fade_pal;
image_ intro_img;
unsigned char **erase_data;
unsigned char **erase_data_cur;
int color_nbr[256 + 1];
unsigned char **(erase_color_ptr[256]);
int i;
int errori;
htimer_t intro_frame_htimer;
htimer_t intro_global_htimer;

/********************************/


static void
img2vram (image_ * image)
{
  fastmem4 (image->buffer, screen, 64000 / 4);
}

static void
copy_vehicle_1 (int x)
{
  int y;
  int dx = 248;
  if (x >= 320)
    dx -= x - 320;
  if (x < 248) {
    dx = x;
    for (y = 31; y >= 0; y--)
      memcpy (screen + 58 * 320 + y * 320,
	      intro_img.buffer + 58 * 320 + 306 - dx + y * 320, dx);
  } else
    for (y = 31; y >= 0; y--)
      memcpy (screen + 58 * 320 + x - 248 + y * 320,
	      intro_img.buffer + 58 * 320 + 58 + y * 320, dx);
}

static void
copy_vehicle_2 (int x)
{
  int y;
  int dx = 248;
  if (x >= 320)
    dx -= x - 320;
  if (x < 248) {
    dx = x;
    for (y = 31; y >= 0; y--)
      memcpy (screen + 110 * 320 + y * 320,
	      intro_img.buffer + 110 * 320 + 262 - dx + y * 320, dx);
  } else
    for (y = 31; y >= 0; y--)
      memcpy (screen + 110 * 320 + x - 248 + y * 320,
	      intro_img.buffer + 110 * 320 + 14 + y * 320, dx);
}

static void
compute_erase_data (void)
{
  unsigned char *dest = screen;
  unsigned char *src = intro_img.buffer;
  int i;
  for (i = 320 * 200; i > 0; i--)
    color_nbr[*src++]++;
  erase_color_ptr[0] = erase_data;
  for (i = 1; i <= 255; i++)
    erase_color_ptr[i] = erase_color_ptr[i - 1] + color_nbr[i - 1];
  src = intro_img.buffer;
  for (i = 320 * 200; i > 0; i--) {
    *erase_color_ptr[*src]++ = dest;
    src++;
    dest++;
  }
}

static unsigned char **
erase (unsigned char **src, int j)
{
  int nbr = color_nbr[j];
  color_nbr[j + 1] += nbr & 1;
  nbr >>= 1;
  while (nbr) {
    char *a = src[0];
    char *b = src[1];
    *a = 0;
    src += 2;
    --nbr;
    *b = 0;
  }
  return src;
}

static void
antialias (unsigned char *src, int nbr)
{
  unsigned int a, b, c, d;
  a = src[-1];
  nbr >>= 1;
  b = src[0];
  do {
    c = src[1];
    ++src;
    a += c;
    d = src[1];
    a >>= 1;
    b += d;
    src[-1] = a;
    b >>= 1;
    a = c;
    src[0] = b;
    b = d;
    ++src;
    --nbr;
  } while (nbr);
}

static char
show_intro (void)
{
  palette_ pal;

  load_soundtrack_from_alias ("INTRO");
  erase_data_cur = erase_data = malloc (64000 * sizeof (char *));
  pcx_load_from_rsc ("intro-logos-img", &intro_img);

  play_soundtrack ();
  memset (color_nbr, 0, 256 * sizeof(*color_nbr));
  set_color (255, 0, 0, 0);
  memset (screen, 255, 32000);
  memset (screen + 32000, 0, 32000);
  reset_htimer (intro_frame_htimer);
  reset_htimer (intro_global_htimer);
  for (i = 0; i <= 63; i += read_htimer (intro_frame_htimer)) {
    set_color (255, i, i, i);
    fade_pal.indiv[255].r = i;
    fade_pal.indiv[255].g = i;
    fade_pal.indiv[255].b = i;
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  for (i = 767; i >= 0; i--)
    fade_pal.global[i] = pal.global[i] = ((i >= 384) ? 63 : 0);

  set_pal ((char *) &pal, 0, 768);
  img2vram (&intro_img);
  while (read_htimer (intro_global_htimer) < 3) {
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  reset_htimer (intro_frame_htimer);
  for (i = 0; i <= 64; i += read_htimer (intro_frame_htimer)) {
    pal2pal ((palette_ *) & pal, &intro_img.palette, i);
    fastmem4 ((char *) &temppal, (char *) &fade_pal, 768 / 4);
    set_pal ((char *) &temppal, 0, 768);
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  set_pal ((char *) &fade_pal, 0, 768);
  vsynch ();

  img_free (&intro_img);
  pcx_load_from_rsc ("intro-vehicles-img", &intro_img);
  while (read_htimer (intro_global_htimer) < 12) {
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  reset_htimer (intro_frame_htimer);
  for (i = 128; i >= 0; i -= read_htimer (intro_frame_htimer)) {
    pal2pal ((palette_ *) & pal, &intro_img.palette, i >> 1);
    fastmem4 ((char *) &temppal, (char *) &fade_pal, 768 / 4);
    set_pal ((char *) &temppal, 0, 768);
    antialias (screen + 85 * 320, 13 * 320);
    antialias (screen + 103 * 320, 13 * 320);
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }

  memset (screen, 255, 32000);
  memset (screen + 32000, 0, 32000);
  set_pal ((char *) &intro_img.palette, 0, 768);

  while (read_htimer (intro_global_htimer) < 18) {
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  
  /* For this sequence, slices will be 4 times shorter */
  intro_frame_htimer->slice_duration /= 4;
  reset_htimer (intro_frame_htimer);
  for (i = 0; i < 568; i += read_htimer (intro_frame_htimer)) {
    copy_vehicle_1 (i);
    copy_vehicle_2 (567 - i);
    vsynch ();
    if (key_or_joy_ready ()) {
      img_free (&intro_img);
      return (1);
    }
  }
  intro_frame_htimer->slice_duration *= 4; /* revert old speed. */

  img_free (&intro_img);
  pcx_load_from_rsc ("intro-splash-img", &intro_img);
  memset (&pal.global, 63, 768);
  vsynch ();
  set_pal ((char *) &pal, 0, 768);
  fastmem4 ((char *) &pal, (char *) &fade_pal, 768 / 4);
  img2vram (&intro_img);
  intro_img.palette.indiv[254].r = 0;
  intro_img.palette.indiv[254].g = 0;
  intro_img.palette.indiv[254].b = 0;
  img_free (&intro_img);	/* will free the picture, not the palette */

  reset_htimer (intro_frame_htimer);
  for (i = 0; i <= 64; i += read_htimer (intro_frame_htimer)) {
    pal2pal ((palette_ *) & pal, &intro_img.palette, i);
    fastmem4 ((char *) &temppal, (char *) &fade_pal, 768 / 4);
    vsynch ();
    set_pal ((char *) &temppal, 0, 768);
    if (key_or_joy_ready ())
      return (1);
  }
  set_pal ((char *) &temppal, 0, 768);

  pcx_load_from_rsc ("intro-erase-img", &intro_img);
  compute_erase_data ();
  img_free (&intro_img);

  while (read_htimer (intro_global_htimer) < 40) {
    vsynch ();
    if (key_or_joy_ready ())
      return (1);
  }

  erase_data_cur = erase_data;
  reset_htimer (intro_frame_htimer);
  for (i = 0; i <= 255; i += read_htimer (intro_frame_htimer)) {
    vsynch ();
    erase_data_cur = erase (erase_data_cur, i);
    if (key_or_joy_ready ())
      return (1);
  }
  return (0);
}

void
play_intro (void)
{
  int i;
  
  intro_frame_htimer = new_htimer (T_LOCAL|T_BLOCKING, HZ (70)); 
  intro_global_htimer = new_htimer (T_GLOBAL, HZ (2)); 

  if (show_intro ()) {
    fastmem4 ((char *) &fade_pal, (char *) &pal, 768 / 4);
    memset ((char *) &pal, 0, 768);
    reset_htimer (intro_frame_htimer);
    for (i = 31; i >= 0; i -= read_htimer (intro_frame_htimer)) {
      pal2pal ((palette_ *) & pal, (palette_ *) & fade_pal, i << 1);
      vsynch ();
      set_pal ((char *) &temppal, 0, 768);
    }
  }
  for (i = 0; i < 768; i++)
    set_color (i, 0, 0, 0);
  free (erase_data);
  unload_soundtrack ();
  while (key_or_joy_ready ())
    get_key_or_joy ();

  free_htimer (intro_frame_htimer);
  free_htimer (intro_global_htimer);
}
