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
#include "errors.h"
#include "pcx.h"
#include "rsc_files.h"
#include "endian.h"
#include "debugmsg.h"

static void
img_init (image_ * image)
{
  image->buffer = malloc (image->size);
  if (image->buffer == NULL)
    fatal_error ("[PCX] Not enough memory.");
}

void
img_free (image_ * image)
{
  free ((char *) image->buffer);
}

static void
delta (image_ * image)
{
  int i;
  char *src = image->buffer + image->width;
  for (i = image->size - image->width; i != 0; i--)
    (*src++) = (char) ((*src) + (*(src - image->width)));
}

char
pcx_load (const char *file, image_ * image)
{
  unsigned long compteur;
  FILE *fptr;

  int nbrbytes, i;
  unsigned char data;

  dmsg (D_FILE, "opening image file: %s", file);

  if ((fptr = fopen (file, "rb")) == NULL) {
    puts (file);
#ifndef __HEDIT__
    fatal_error ("Unable to open this PCX\n");
#else
    fatalog ("Unable to open this PCX\n");
#endif
  }
  fread (&(image->header), sizeof (header_), 1, fptr);

  /* convert to local endianess */
  image->header.x = BSWAP16 (image->header.x);
  image->header.y = BSWAP16 (image->header.y);
  image->header.width = BSWAP16 (image->header.width);
  image->header.height = BSWAP16 (image->header.height);
  image->header.widthdpi = BSWAP16 (image->header.widthdpi);
  image->header.heightdpi = BSWAP16 (image->header.heightdpi);
  image->header.bytes_per_lines = BSWAP16 (image->header.bytes_per_lines);
  image->header.palette_kind = BSWAP16 (image->header.palette_kind);

  image->width = (image->header.width - image->header.x + 1);
  image->height = (image->header.height - image->header.y + 1);
  image->size = image->width * image->height;

  dmsg (D_FILE, "size=(%d,%d) rle=%d", 
	image->header.width + 1, image->header.height + 1, image->header.rle);

  img_init (image);

  compteur = 0;
  if (image->header.rle)
    while (compteur < image->size) {
      data = (unsigned char) getc (fptr);
      if ((data & 192) == 192) {
	nbrbytes = data & 63;
	data = (unsigned char) getc (fptr);
	while (nbrbytes--)
	  image->buffer[compteur++] = data;
      } else
	image->buffer[compteur++] = data;
  } else
    fread (image->buffer, image->size, 1, fptr);
  if (image->header.rle == 2)
    delta (image);


  data = (unsigned char) getc (fptr);	// data==0Ch expected

  fread (&(image->palette), 768, 1, fptr);
  for (i = 0; i < 256 * 3; i++)
    image->palette.global[i] =
      (unsigned char) (image->palette.global[i] >> 2);

  fclose (fptr);
  return (0);
}

char 
pcx_load_from_rsc (const char *rsc, image_ * image)
{
  char* res = get_rsc_file (rsc);
  char error;

  if (res == 0) 
    fatal_error ("Empty resource.\n");
  error = pcx_load (res, image);
  free (res);
  return error;
}
