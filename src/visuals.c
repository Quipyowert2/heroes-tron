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

#include "common.h"
#include "const.h"
#include "display.h"
#include "fastmem.h"
#include "visuals.h"
#include "endian.h"
#include "timer.h"
#include "heroes.h"

int rotosinus[256];
int rotocosinus[256];
int rotosinus2[256];
int rotocosinus2[256];
signed char moyensinus[512];
int mulxbuf[200];
int angle;
pixel_t *srcroto;

void
rotozoom_buffer (void)
{
  pixel_t *dest = render_buffer[1] + xbuf;
  int x, y = 200;
  int debutx, debuty;
  int inx, iny;
  int deltax, deltay;
  int deltax2, deltay2;

  srcroto = corner[0];
  angle = (player[col2plr[0]].rotozoom & 511) >> 1;
  if (player[col2plr[0]].rotozoom_direction)
    angle = (-angle) & 255;
  deltay = rotosinus[angle];	/* 2; */
  deltax = rotocosinus[angle];	/* 2; */
  deltay2 = rotosinus2[angle];	/* 2; */
  deltax2 = rotocosinus2[angle];	/* 2; */
  debutx = (160 << 16) - 160 * deltax - 100 * deltax2;
  debuty = (100 << 16) - 160 * deltay - 100 * deltay2;

  do {
    inx = debutx;
    iny = debuty;
    x = 320;

    if ((unsigned) inx >= (320 << 16) || (unsigned) iny >= (200 << 16))
      do {
	*dest++ = 0;
	inx += deltax;
	iny += deltay;
	x--;
      } while ((unsigned) inx >= (320 << 16)
	       || (unsigned) iny >= (200 << 16));
    do {
      *dest++ =
	*(srcroto + (((unsigned) inx) >> 16) +
	  mulxbuf[((unsigned) iny) >> 16]);
      x--;
      inx += deltax;
      iny += deltay;

    } while ((unsigned) inx < (320 << 16) && (unsigned) iny < (200 << 16)
	     && x > 0);
    if (x > 0)
      do {
	*dest++ = 0;
	x--;
      } while (x > 0);

    debutx += deltax2;
    debuty += deltay2;
    dest += xbuf - 320;
  } while (--y);
}

void
rotozoom_half_buffer (int c)
{
  pixel_t *dest = render_buffer[c] + xbuf - 180 + xbuf;
  int x, y = 200;
  int debutx, debuty;
  int inx, iny;
  int deltax, deltay;
  int deltax2, deltay2;

  srcroto = corner[c];

  angle = (player[col2plr[c]].rotozoom & 511) >> 1;
  if (player[col2plr[0]].rotozoom_direction)
    angle = (-angle) & 255;
  deltay = rotosinus[angle];	/* 2; */
  deltax = rotocosinus[angle];	/* 2; */
  deltay2 = rotosinus2[angle];	/* 2; */
  deltax2 = rotocosinus2[angle];	/* 2; */
  debutx = (80 << 16) - 80 * deltax - 100 * deltax2;
  debuty = (100 << 16) - 80 * deltay - 100 * deltay2;

  do {
    inx = debutx;
    iny = debuty;
    x = 160;

    if ((unsigned) inx >= (160 << 16) || (unsigned) iny >= (200 << 16))
      do {
	*dest++ = 0;
	inx += deltax;
	iny += deltay;
	x--;
	if (x <= 0)
	  goto endline;
      } while ((unsigned) inx >= (160 << 16)
	       || (unsigned) iny >= (200 << 16));
    do {
      *dest++ =
	*(srcroto + (((unsigned) inx) >> 16) +
	  mulxbuf[((unsigned) iny) >> 16]);
      x--;
      if (x <= 0)
	goto endline;
      inx += deltax;
      iny += deltay;

    } while ((unsigned) inx < (160 << 16) && (unsigned) iny < (200 << 16)
	     && x > 0);
    if (x > 0)
      do {
	*dest++ = 0;
	x--;
      } while (x > 0);
  endline:
    debutx += deltax2;
    debuty += deltay2;
    dest += xbuf - 160;
  } while (--y);
}

static void
horizontal_zoom_wave (pixel_t *src, pixel_t *dest, int oldsize, int newsize)
{
  unsigned long int x = 0, deltax = ((1 + oldsize) << 16) / (newsize);
  u32_t *ad;
  u32_t tmp;
  pixel_t tmp1;
  u32_t *adest;

  if (((int) dest) & 3)
    do {
      tmp1 = *(src + (x >> 16));
      x += deltax;
      *dest++ = tmp1;
      newsize--;
    } while (((int) dest) & 3);
  ad = (u32_t*) (src + (x >> 16));
  tmp = GETWORD((u8_t*)ad);
  adest = (u32_t*) dest;	/* adest utilisé dans cette boucle seulement
				   pour que Watcom le garde dans un registre */
  do {
    *adest = tmp;
    adest++;
    x += deltax * 4;
    ad = (u32_t*) (src + (x >> 16));
    newsize -= 4;
    tmp = GETWORD((u8_t*)ad);
  } while (newsize > 0);
}

static void
horizontal_zoom_flip (pixel_t *src, pixel_t *dest, int oldsize, int newsize)
{
  unsigned long int x = 0, deltax = ((1 + oldsize) << 16) / (newsize);
  u16_t *ad;
  u32_t tmp;
  pixel_t tmp1;
  u16_t *adest;

  if (((int) dest) & 1)
    do {
      tmp1 = *(src + (x >> 16));
      x += deltax;
      *dest++ = tmp1;
      newsize--;
    } while (((int) dest) & 1);
  ad = (u16_t *) (src + (x >> 16));
  tmp = GETWORD((u8_t *)ad);
  adest = (u16_t *) dest;	/* adest utilisé dans cette boucle seulement
				   pour que Watcom le garde dans un registre */
  do {
    *adest = tmp;
    adest++;
    x += deltax * 2;
    ad = (u16_t *) (src + (x >> 16));
    newsize -= 2;
    tmp = GETWORD((u8_t *)ad);
  } while (newsize > 0);
}

static void
vertical_zoom_wave (pixel_t *src, pixel_t *dest, int oldsize, int newsize)
{
  unsigned long int y = 0, deltay = ((1 + oldsize) << 16) / (newsize);
  u32_t tmp1, tmp2;
  newsize--;
  do {
    pixel_t *p;
    p = src + ((y >> 16) * 3 << 7);
    tmp1 = GETWORD(p);
    p = src + (((y + deltay) >> 16) * 3 << 7);
    tmp2 = GETWORD(p);
    y += deltay << 1;
    *(u32_t *) dest = tmp1;
    *(u32_t *) (dest + xbuf) = tmp2;
    dest += xbuf * 2;
    newsize -= 2;
  } while (newsize > 0);
  if (newsize == 0) {
    pixel_t *p;
    p = src + ((y >> 16) * 3 << 7);
    tmp1 = GETWORD(p);
    *(u32_t *) dest = tmp1;
  }
}

static int
which_line (int y, int a)
{
  float an;
  a &= 1023;
  an = a * 2 * 3.1415926535 / 1024;
  return (ceil ((200.0 * y) / (200.0 * cos (an) - y * sin (an))));
}

static int
which_column (int y, int a)
{
  float an;
  a &= 1023;
  an = a * 2 * 3.1415926535 / 1024;
  return (ceil (-4 * (sin (an) * y + 200) / 5));
}
static int
which_offset (int y, int a)
{
  float an;
  a &= 1023;
  an = a * 2 * 3.1415926535 / 1024;
  return (ceil ((-160 * 200) / (y * sin (an) + 200)));
}

void
wave_buffer (void)
{
  int i, p = player[col2plr[0]].waves * 7, gauche, droite, j;
  int waves_begin = player[col2plr[0]].waves_begin;
  for (i = 0; i < 320; i += 4) {
    gauche = ((16 + moyensinus[(i * 2 + p) & 511]) * waves_begin) / 128;
    droite = ((16 + moyensinus[(i + p) & 511]) * waves_begin) / 128;
    for (j = 0; j < gauche; j++)
      *(u32_t *) (render_buffer[1] + i + j * xbuf) = 0;
    vertical_zoom_wave (corner[0] + i, render_buffer[1] + i + gauche * xbuf,
			200, 200 - droite - gauche);
    for (j = 0; j < droite; j++)
      *(u32_t *) (render_buffer[1] + i + (200 - j) * xbuf) = 0;
  }
  for (i = 0; i < 200; i++) {
    gauche = ((16 + moyensinus[(i * 2 + p) & 511]) * waves_begin) / 128;
    droite = ((16 + moyensinus[(i - p) & 511]) * waves_begin) / 128;
    for (j = 0; j < gauche; j++)
      *(render_buffer[0] + i * xbuf + j) = 0;
    horizontal_zoom_wave (render_buffer[1] + i * xbuf,
			  render_buffer[0] + i * xbuf + gauche, 320,
			  320 - droite - gauche);
    for (j = 0; j < droite; j++)
      *(render_buffer[0] + i * xbuf + 319 - j) = 0;
  }
}

void
wave_half_buffer (int c)
{
  int i, p = player[col2plr[c]].waves * 7, gauche, droite, j;
  int waves_begin = player[col2plr[c]].waves_begin;
  for (i = 0; i < 160; i += 4) {
    gauche = ((16 + moyensinus[(i * 2 + p) & 511]) * waves_begin) / 128;;
    droite = ((16 + moyensinus[(i + p) & 511]) * waves_begin) / 128;;
    for (j = 0; j < gauche; j++)
      *(u32_t *) (render_buffer[c] + 200 + i + j * xbuf) = 0;
    vertical_zoom_wave (corner[c] + i,
			render_buffer[c] + 200 + i + gauche * xbuf, 200,
			200 - droite - gauche);
    for (j = 0; j < droite; j++)
      *(u32_t *) (render_buffer[c] + 200 + i + (200 - j) * xbuf) = 0;
  }
  for (i = 0; i < 200; i++) {
    gauche = ((16 + moyensinus[(i * 2 + p) & 511]) * waves_begin) / 128;;
    droite = ((16 + moyensinus[(i - p) & 511]) * waves_begin) / 128;;
    for (j = 0; j < gauche; j++)
      *(render_buffer[c] + i * xbuf + j) = 0;
    horizontal_zoom_wave (render_buffer[c] + 200 + i * xbuf,
			  render_buffer[c] + i * xbuf + gauche, 160,
			  160 - droite - gauche);
    for (j = 0; j < droite; j++)
      *(render_buffer[c] + i * xbuf + 159 - j) = 0;
  }
}

void
flip_buffer (int p2)
{
  int i, y, gauche, j;
  for (i = 0; i < 200; i++) {
    y = which_line (i - 100, p2);
    if (y >= -100 && y < 100) {
      gauche = which_offset (y, p2) + 160;
      if (gauche >= 0) {
	y += 100;
	for (j = 0; j < gauche; j++)
	  *(render_buffer[1] + i * xbuf + j) = 0;
	horizontal_zoom_wave (corner[0] + y * xbuf,
			      render_buffer[1] + i * xbuf + gauche, 320,
			      320 - gauche - gauche);
	for (j = 320 - gauche; j < 320; j++)
	  *(render_buffer[1] + i * xbuf + j) = 0;
      } else {
	gauche = which_column (y, p2) + 160;
	y += 100;
	horizontal_zoom_flip (corner[0] + y * xbuf + gauche,
			      render_buffer[1] + i * xbuf,
			      320 - gauche - gauche, 320);
      }
    } else {
      memset (render_buffer[1] + i * xbuf, 0, 320);
    }
  }
}

#define corner_buffer_glenz 1
static void
corner_buffer_begin (int i)
{
  int j, k, l;
  pixel_t *src;
  pixel_t *dest;
  pixel_t *dest2;
  pixel_t *glenzl = glenz[corner_buffer_glenz];
  pixel_t *glenzd = glenz[0];

  dest2 = corner[0] + xbuf * (200 - i - 1);
  for (j = 200 - i - 1, l = 0; j < 200; j++, l++) {
    src = corner[0] + j * xbuf + l;
    dest = corner[0] + j * xbuf;
    for (k = l; k != 0; k--)
      *dest++ = 0;
    for (k = i + 1; k != 0; k--) {
      *dest = glenzl[*src];
      src += xbuf;
      dest++;
    }
    if (l > 9) {
      dest[0] = glenzd[dest[0]];
      dest[1] = glenzd[dest[1]];
      dest[2] = glenzd[dest[2]];
      dest[3] = glenzd[dest[3]];
      dest[4] = glenzd[dest[4]];
      dest[5] = glenzd[dest[5]];
      dest[6] = glenzd[dest[6]];
      dest[7] = glenzd[dest[7]];
      dest[8] = glenzd[dest[8]];
      dest[9] = glenzd[dest[9]];
      dest[10] = glenzd[dest[10]];
    }
    i--;
  }
  for (j = 200 - i - 1, l = 0; j <= 200; j++, l++) {
    dest2[-xbuf] = glenzd[dest2[-xbuf]];
    *dest2 = glenzd[glenzd[*dest2]];
    dest2 += xbuf + 1;
  }
}

static void
corner_buffer_middle (int i)
{
  int j, k, l;
  pixel_t *src;
  pixel_t *dest;
  pixel_t *dest2;
  pixel_t *glenzl = glenz[corner_buffer_glenz];
  pixel_t *glenzd = glenz[0];

  i -= 199;
  dest2 = corner[0] + i;
  for (j = 0, l = 0; j < 200; j++, l++) {
    src = corner[0] + j * xbuf + i + l;
    dest = corner[0] + j * xbuf;
    for (k = i + l; k != 0; k--)
      *dest++ = 0;
    for (k = 200 - l; k != 0; k--) {
      *dest = glenzl[*src];
      src += xbuf;
      dest++;
    }
    if (l > 9) {
      dest[0] = glenzd[dest[0]];
      dest[1] = glenzd[dest[1]];
      dest[2] = glenzd[dest[2]];
      dest[3] = glenzd[dest[3]];
      dest[4] = glenzd[dest[4]];
      dest[5] = glenzd[dest[5]];
      dest[6] = glenzd[dest[6]];
      dest[7] = glenzd[dest[7]];
      dest[8] = glenzd[dest[8]];
      dest[9] = glenzd[dest[9]];
      dest[10] = glenzd[dest[10]];
    }
  }
  for (l = 0; l < 200; l++) {
    dest2[2] = glenzd[dest2[2]];
    *dest2 = glenzd[glenzd[*dest2]];
    dest2 += xbuf + 1;
  }
}

static void
corner_buffer_end (int i)
{
  int j, k, l;
  pixel_t *src;
  pixel_t *dest;
  pixel_t *dest2;
  pixel_t *glenzl = glenz[corner_buffer_glenz];
  pixel_t *glenzd = glenz[0];

  i -= 319;
  dest2 = corner[0] + i + 120;
  for (j = 0, l = 120; j < 200; j++, l++) {
    dest = corner[0] + j * xbuf;
    if (i + l < 320) {
      src = corner[0] + j * xbuf + i + l;
      for (k = i + l; k != 0; k--)
	*dest++ = 0;
      for (k = 320 - l - i; k != 0; k--) {
	*dest = glenzl[*src];
	src += xbuf;
	dest++;
      }
    } else 
      memset (dest, 0, 320);
  }
  for (l = 0; i + l < 200; l++) {
    dest2[1] = glenzd[dest2[1]];
    *dest2 = glenzd[glenzd[*dest2]];
    dest2 += xbuf + 1;
  }
}

extern int
corner_buffer (void)
{
  int t2 = read_htimer (corner_htimer);

  if (t2 < 200)
    corner_buffer_begin (t2);
  else if (t2 < 320)
    corner_buffer_middle (t2);
  else if (t2 < 520)
    corner_buffer_end (t2);
  else 
    return 1;
  return 0;
}

extern void
compute_lut (void)
{
  int i;
  for (i = 0; i != 256; i++) {
    rotosinus[i] = ceil (sin (i * 3.141592653 / 128.0) * 65536 / 1.2);
    rotocosinus[i] = ceil (cos (i * 3.141592653 / 128.0) * 65536);
    rotosinus2[i] =
      ceil (sin (i * 3.141592653 / 128.0 + 3.141592653 / 2.0) * 65536);
    rotocosinus2[i] =
      ceil (cos (i * 3.141592653 / 128.0 + 3.141592653 / 2.0) * 65536 * 1.2);
  }
  for (i = 0; i != 512; i++)
    moyensinus[i] = ceil (sin (i * 2.0 * 3.141592653 / 512.0) * 16.2);
  for (i = 0; i != 200; i++)
    mulxbuf[i] = i * xbuf;
}
