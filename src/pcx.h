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

typedef struct pcx_header_type
{
  char signature;
  char version;
  char rle;
  char bits_per_pixels;
  short int x, y;
  short int width, height;
  short int widthdpi, heightdpi;
  char egapal[48];
  char inutil;
  char nbrplanes;
  short int bytes_per_lines;
  short int palette_kind;
  char rien[58];
}
pcx_header_t ATTRIBUTE_PACKED;

typedef struct
{
  unsigned char r, g, b;
}
color_rvb_t;

typedef union
{
  color_rvb_t indiv[256];
  unsigned char global[256 * 3];
}
palette_t;

typedef struct img_type
{
  pcx_header_t header;
  palette_t palette;
  unsigned int width, height;
  unsigned int size;
  char *buffer;
}
pcx_image_t;

void img_free (pcx_image_t * image);
char pcx_load (const char *file, pcx_image_t * image);
char pcx_load_from_rsc (const char *rsc, pcx_image_t * image);

#endif /* HEROES__PCX__H */
