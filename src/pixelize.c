/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License as published by |
| the Free Software Foundation; either version 2 of the License, or |
| (at your option) any later version.                               |
|                                                                   |
| Heroes is distributed in the hope that it will be useful, but     |
| WITHOUT ANY WARRANTY; without even the implied warranty of        |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU |
| General Public License for more details.                          |
|                                                                   |
| You should have received a copy of the GNU General Public License |
| along with this program; if not, write to the Free Software       |
| Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          |
| 02111-1307 USA                                                    |
`------------------------------------------------------------------*/

#include "system.h"
#include "display.h"
#include "fastmem.h"
#include "const.h"
#include "pixelize.h"

static void
pixelize_1 (pixel_t *dd, const pixel_t *ss)
{
  pixel_t *dest = dd;
  const pixel_t *src = ss;
  int i;

  for (i = 200; i != 0; i--, src += xbuf, dest += xbuf)
    fastmem4 (src, dest, 80);
}

static void
pixelize_2_inline (const pixel_t *s, pixel_t *d)
{
  int y, x;
  for (y = 200 / 2; y; --y) {
    for (x = 320 / 4; x; --x) {
      pixel_t t1, t2;
      t1 = s[0];
      t2 = s[1];
      d[0] = t1;
      d[2] = t2;
      d[1] = t1;
      d[3] = t2;
      d[xbuf + 0] = t1;
      d[xbuf + 2] = t2;
      d[xbuf + 1] = t1;
      d[xbuf + 3] = t2;
      s += 4;
      d += 4;
    }
    s += 2 * xbuf - 320;
    d += 2 * xbuf - 320;
  }
}


static void
pixelize_2 (pixel_t *dd, const pixel_t *ss)
{
  pixelize_2_inline (ss, dd);
}

static void
pixelize_4_inline (const pixel_t *s, int *d)
{
  int y, x;
  for (y = 200 / 4; y; --y) {
    for (x = 320 / 4; x; --x) {
      pixel_t t1 = *s;
      int t;
      t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
      d[0] = t;
      d[xbuf / 4] = t;
      d[2 * xbuf / 4] = t;
      d[3 * xbuf / 4] = t;
      s += 4;
      d += 1;
    }
    s += 4 * xbuf - 320;
    d += (4 * xbuf - 320) / 4;
  }
}

static void
pixelize_4 (pixel_t *dd, const pixel_t *ss)
{
  int *dest = ((int *) dd);
  const pixel_t *src = ss;

  pixelize_4_inline (src, dest);
}

static void
pixelize_8_inline (const pixel_t *s, int *d)
{
  int y, x;
  for (y = 200 / 8; y; --y) {
    for (x = 320 / 8; x; --x) {
      pixel_t t1 = *s;
      int t;
      t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
      d[0] = t;
      d[1] = t;
      d[xbuf / 4] = t;
      d[xbuf / 4 + 1] = t;
      d[2 * xbuf / 4] = t;
      d[2 * xbuf / 4 + 1] = t;
      d[3 * xbuf / 4] = t;
      d[3 * xbuf / 4 + 1] = t;
      d[4 * xbuf / 4] = t;
      d[4 * xbuf / 4 + 1] = t;
      d[5 * xbuf / 4] = t;
      d[5 * xbuf / 4 + 1] = t;
      d[6 * xbuf / 4] = t;
      d[6 * xbuf / 4 + 1] = t;
      d[7 * xbuf / 4] = t;
      d[7 * xbuf / 4 + 1] = t;
      s += 8;
      d += 2;
    }
    s += 8 * xbuf - 320;
    d += (8 * xbuf - 320) / 4;
  }
}

static void
pixelize_8 (pixel_t *dd, const pixel_t *ss)
{
  int *dest = ((int *) dd);
  const pixel_t *src = ss;

  pixelize_8_inline (src, dest);
}

static void
pixelize_16_inline (const pixel_t *s, int *d)
{
  int y, x, o;
  for (y = 200 / 16; y; --y) {
    for (x = 320 / 16; x; --x) {
      pixel_t t1 = *s;
      int t;
      t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
      for (o = 16 * xbuf / 4; o >= 0; o -= xbuf / 4) {
	d[o + 0] = t;
	d[o + 1] = t;
	d[o + 2] = t;
	d[o + 3] = t;
      }
      s += 16;
      d += 4;
    }
    s += 16 * xbuf - 320;
    d += (16 * xbuf - 320) / 4;
  }
}

static void
pixel_16_inline_2 (const pixel_t *s, int *d)
{
  int x, o;
  for (x = 320 / 16; x; --x) {
    pixel_t t1 = *s;
    int t;
    t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
    for (o = (200 % 16 - 1) * xbuf / 4; o >= 0; o -= xbuf / 4) {
      d[o + 0] = t;
      d[o + 1] = t;
      d[o + 2] = t;
      d[o + 3] = t;
    }
    s += 16;
    d += 4;
  }
}

static void
pixelize_16 (pixel_t *dd, const pixel_t *ss)
{
  int *dest = ((int *) dd);
  const pixel_t *src = ss;

  pixelize_16_inline (src, dest);
  pixel_16_inline_2 (src + xbuf * 192, dest + (xbuf * 192) / 4);
}

static void
pixelize_32_inline (const pixel_t *s, int *d)
{
  int y, x, o;
  for (y = 200 / 32; y; --y) {
    for (x = 320 / 32; x; --x) {
      pixel_t t1 = *s;
      int t;
      t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
      for (o = 32 * xbuf / 4; o >= 0; o -= xbuf / 4) {
	d[o + 0] = t;
	d[o + 1] = t;
	d[o + 2] = t;
	d[o + 3] = t;
	d[o + 4] = t;
	d[o + 5] = t;
	d[o + 6] = t;
	d[o + 7] = t;
      }
      s += 32;
      d += 8;
    }
    s += 32 * xbuf - 320;
    d += (32 * xbuf - 320) / 4;
  }
}

static void
pixel_32_inline_2 (const pixel_t *s, int *d)
{
  int x, o;
  for (x = 320 / 32; x; --x) {
    pixel_t t1 = *s;
    int t;
    t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
    for (o = (200 % 32 - 1) * xbuf / 4; o >= 0; o -= xbuf / 4) {
      d[o + 0] = t;
      d[o + 1] = t;
      d[o + 2] = t;
      d[o + 3] = t;
      d[o + 4] = t;
      d[o + 5] = t;
      d[o + 6] = t;
      d[o + 7] = t;
    }
    s += 32;
    d += 8;
  }
}

static void
pixelize_32 (pixel_t *dd, const pixel_t *ss)
{
  int *dest = ((int *) dd);
  const pixel_t *src = ss;

  pixelize_32_inline (src, dest);
  pixel_32_inline_2 (src + xbuf * 192, dest + (xbuf * 192) / 4);
}

static void
pixelize_64_inline (const pixel_t *s, int *d)
{
  int y, x, o;
  for (y = 200 / 64; y; --y) {
    for (x = 320 / 64; x; --x) {
      pixel_t t1 = *s;
      int t;
      t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
      for (o = 64 * xbuf / 4; o >= 0; o -= xbuf / 4) {
	d[o + 0] = t;
	d[o + 1] = t;
	d[o + 2] = t;
	d[o + 3] = t;
	d[o + 4] = t;
	d[o + 5] = t;
	d[o + 6] = t;
	d[o + 7] = t;
	d[o + 8] = t;
	d[o + 9] = t;
	d[o + 10] = t;
	d[o + 11] = t;
	d[o + 12] = t;
	d[o + 13] = t;
	d[o + 14] = t;
	d[o + 15] = t;
      }
      s += 64;
      d += 16;
    }
    s += 64 * xbuf - 320;
    d += (64 * xbuf- 320) / 4;
  }
}

static void
pixel_64_inline_2 (const pixel_t *s, int *d)
{
  int x, o;
  for (x = 320 / 64; x; --x) {
    pixel_t t1 = *s;
    int t;
    t = t1 | (t1 << 8) | (t1 << 16) | (t1 << 24);
    for (o = (200 % 64 - 1) * xbuf / 4; o >= 0; o -= xbuf / 4) {
      d[o + 0] = t;
      d[o + 1] = t;
      d[o + 2] = t;
      d[o + 3] = t;
      d[o + 4] = t;
      d[o + 5] = t;
      d[o + 6] = t;
      d[o + 7] = t;
      d[o + 8] = t;
      d[o + 9] = t;
      d[o + 10] = t;
      d[o + 11] = t;
      d[o + 12] = t;
      d[o + 13] = t;
      d[o + 14] = t;
      d[o + 15] = t;
    }
    s += 64;
    d += 16;
  }
}

static void
pixelize_64 (pixel_t *dest, const pixel_t *src)
{
  int *d = ((int *) dest);
  const pixel_t *s = src;

  pixelize_64_inline (s, d);
  pixel_64_inline_2 (s + xbuf * 192, d + (xbuf * 192) / 4);
}

void (*pixelize[7]) (pixel_t *, const pixel_t *) =
{
  pixelize_1,
  pixelize_2,
  pixelize_4,
  pixelize_8,
  pixelize_16,
  pixelize_32,
  pixelize_64
};
