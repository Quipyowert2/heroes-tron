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
header_ ATTRIBUTE_PACKED;

typedef struct color_rgb
{
  unsigned char r, g, b;
}
color_;

typedef union			/*palette_rvb */
{
  color_ indiv[256];
  unsigned char global[256 * 3];
}
palette_, palette_rvb;

typedef struct img_type
{
  header_ header;
  palette_ palette;
  unsigned int width, height;
  unsigned int size;
  char *buffer;
}
image_;

void img_free (image_ * image);
char pcx_load (const char *file, image_ * image);
char pcx_load_from_rsc (const char *rsc, image_ * image);

#endif /* HEROES__PCX__H */
