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
#include "sprunish.h"

void
draw_sprunish_custom (const sprite_t *sprite, pixel_t *dest, pixel_t color)
{
  pixel_t	*cur = dest;	/* current writting possition */
  u8_t		*pc;		/* program counter */
  u8_t		*epc;		/* end of program code */

  assert (sprite->all.kind == S_RLE_UNIC_SHADE);

  pc = sprite->unish.code;
  epc = sprite->unish.end_code;

  while (pc < epc) {
    unsigned m, n;
    m = *pc++;
    n = *pc++;
    if (n == 0 && m == 0 && *pc == 0) {	/* end of line */
      ++pc;
      cur += sprite->unish.line_skip;
    } else {
      /* skip transparent pixels */
      cur += m;
      /* draw the opaque pixels */
      for (; n; --n)
	*cur++ = color;
      /* draw the glenz pixels */
      for (m = *pc++; m; --m) {
	*cur = sprite->shade.glenz[*cur];
	++cur;
      }
    }
  }
}

void
draw_sprunish (const sprite_t *sprite, pixel_t *dest)
{
  assert (sprite->all.kind == S_RLE_UNIC_SHADE);
  draw_sprunish_custom (sprite, dest, sprite->unish.color);
}


sprite_t *
compile_sprunish (const pixel_t *src, pixel_t transp_color,
		  pixel_t glenz_color, pixel_t *glenz_line,
		  pixel_t opaque_color,
		  unsigned int block_height,
		  unsigned int block_width,
		  unsigned int src_width, unsigned int dest_width)
{
  sprite_t* sprite;
  unsigned int row;
  unsigned int code_size;
  u8_t *pc;			/* program counter */


  /* In the worst case (start with an opaque pixel and alternate
     transparant and opaque), we need three bytes to encode two
     pixels, plus three bytes for the first pixel, and three for the end
     of line. */
  code_size = block_height * ((block_width / 2 + 1) * 3 + 3);

  XMALLOC_VAR (sprite);
  XMALLOC_ARRAY (sprite->unish.code, code_size);

  /* encode the bloc */
  pc = sprite->unish.code;
  for (row = block_height; row; --row) {
    unsigned int m, n, s;
    const pixel_t *eol = src + block_width; /* end of line */

    /* encode a line */
    do {
      /* count the number of transparant pixels */
      for (m = 0; *src == transp_color && src < eol && m < 255; ++src)
	++m;
      /* count the number of opaque pixels */
      for (n = 0; *src != transp_color && *src != glenz_color
	     && src < eol && n < 255; ++src)
	++n;
      /* count the number of glenz pixels */
      for (s = 0; *src == glenz_color && src < eol && s < 255; ++src)
	++s;
      /* write the corresponding data */
      *pc++ = m;      *pc++ = n;
      *pc++ = s;

    } while (src < eol);
    /* output an end of line */
    *pc++ = 0;
    *pc++ = 0;
    *pc++ = 0;

    /* prepare for next line */
    src += src_width - block_width;
  }

  sprite->unish.kind = S_RLE_UNIC_SHADE;
  sprite->unish.draw = draw_sprunish;
  sprite->unish.end_code = pc;
  sprite->unish.line_skip = dest_width - block_width;
  sprite->unish.glenz = glenz_line;
  sprite->unish.color = opaque_color;

  assert (pc < sprite->unish.code + code_size);

  return sprite;
}

void free_sprunish (sprite_t *prog)
{
  assert (prog->all.kind == S_RLE_UNIC_SHADE);
  free (prog->unish.code);
  free (prog);
}
