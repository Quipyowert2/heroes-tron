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

#ifndef HEROES__SCRTOOLS__H
#define HEROES__SCRTOOLS__H

#include "pcx.h"

void set_pal_with_luminance (const palette_t* palsrc);
void set_palette (const unsigned char *palette);

void flush_display (const pixel_t *src);
void flush_display2 (const pixel_t *src1, const pixel_t *src2);
void flush_display_moving (int x);
void flush_display2_moving (int x);

void shade_scr_area (const pixel_t *src, pixel_t *dest);
void copy_scr_area (const pixel_t *src, pixel_t *dest);
void copy_image_to_scr_area (const pcx_image_t *src, pixel_t *dest);
void clear_scr_area (pixel_t *dest);

#endif /* HEROES__SCRTOOLS__H */
