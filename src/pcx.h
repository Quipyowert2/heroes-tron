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


#ifndef HEROES__PCX__H
#define HEROES__PCX__H

#include "display.h"

typedef struct pcx_header_type
{
  u8_t signature;
  u8_t version;
  u8_t rle;
  char bits_per_pixels;
  u16_t x, y;
  u16_t width, height;
  u16_t widthdpi, heightdpi;
  u8_t egapal[48];
  u8_t inutil;
  u8_t nbrplanes;
  u16_t bytes_per_lines;
  u16_t palette_kind;
  u8_t rien[58];
}
pcx_header_t ATTRIBUTE_PACKED;

typedef struct
{
  u8_t r, g, b;
}
color_rvb_t;

typedef union
{
  color_rvb_t indiv[256];
  u8_t global[256 * 3];
}
palette_t;

typedef struct img_type
{
  pcx_header_t header;
  palette_t palette;
  unsigned int width, height;
  unsigned int size;
  pixel_t *buffer;
}
pcx_image_t;

void img_free (pcx_image_t * image);
char pcx_load (const char *file, pcx_image_t * image);
char pcx_load_from_rsc (const char *rsc, pcx_image_t * image);

#define IMGPOS(img,row,col) ((img).buffer + (row) * (img).width + (col))

#endif /* HEROES__PCX__H */
