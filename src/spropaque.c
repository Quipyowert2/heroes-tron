/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
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
#include "spropaque.h"

void
draw_spropaque (const sprite_t *sprite, pixel_t *dest)
{
  pixel_t	*data;		/* input position */
  pixel_t	*edata;		/* end of input */
  int		width;
  int		lskip;

  assert (sprite->all.kind == S_OPAQUE);

  data = sprite->opaq.data;
  edata = sprite->opaq.end_data;
  width = sprite->opaq.width;
  lskip = sprite->opaq.line_skip;

  while (data < edata) {
    int i = width;
    while (i--)
      *dest++ = *data++;
    dest += lskip;
  }
}

sprite_t *
compile_spropaque (const pixel_t *src,
		   unsigned int block_height, unsigned int block_width,
		   unsigned int src_width, unsigned int dest_width)
{
  sprite_t *sprite;
  unsigned int row;
  unsigned int data_size;
  pixel_t *data;

  data_size = block_height * block_width;

  XMALLOC_VAR (sprite);
  XMALLOC_ARRAY (sprite->opaq.data, data_size);
  data = sprite->opaq.data;

  for (row = block_height; row; --row) {
    memcpy (data, src, block_width);
    src += src_width;
    data += block_width;
  }

  sprite->opaq.kind = S_OPAQUE;
  sprite->opaq.draw = draw_spropaque;
  sprite->opaq.end_data = data;
  sprite->opaq.width = block_width;
  sprite->opaq.line_skip = dest_width - block_width;

  return sprite;
}

void
free_spropaque (sprite_t *prog)
{
  assert (prog->all.kind == S_OPAQUE);
  free (prog->opaq.data);
  free (prog);
}
