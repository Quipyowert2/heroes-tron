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
aff_buffer (void)
{
  unsigned char *src = corner[0];
  unsigned char *dest = (char *) screen;
  int i;

  for (i = 200; i > 0; i--, src += xbuf, dest += 320)
    fastmem4 (src, dest, 320 / 4);
}
