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
#include "sprglenz.h"

void draw_sprglenz_custom (const sprite_t *sprite, pixel_t *dest,
			   const pixel_t *glenz_line)
{
  pixel_t	*cur = dest;	/* current writting possition */
  u8_t		*pc;		/* program counter */
  u8_t		*epc;		/* end of program code */

  assert (sprite->all.kind == S_RLE_GLENZ);

  pc = sprite->glenz.code;
  epc = sprite->glenz.end_code;

  while (pc < epc) {
    unsigned m, n;
    m = *pc++;
    n = *pc++;
    if (n == 0 && m == 0) {	/* end of line */
      cur += sprite->glenz.line_skip;
    } else {
      /* skip transparent pixels */
      cur += m;
      /* draw the glenz pixels */
      for (; n; --n) {
	*cur = glenz_line[*cur];
	++cur;
      }
    }
  }
}

void draw_sprglenz (const sprite_t *sprite, pixel_t *dest)
{
  assert (sprite->all.kind == S_RLE_GLENZ);
  draw_sprglenz_custom (sprite, dest, sprite->glenz.glenz);
}

sprite_t *compile_sprglenz (const pixel_t *src, pixel_t transp_color,
			    pixel_t *glenz_line,
			    unsigned int block_height,
			    unsigned int block_width,
			    unsigned int src_width, unsigned int dest_width)
{
  sprite_t* sprite;
  unsigned int row;
  unsigned int code_size;
  u8_t *pc;			/* program counter */


  /* In the worst case (start with an opaque pixel and alternate
     transparant and opaque), we need two bytes to encode two
     pixels, plus two bytes for the first pixel, and two for the end
     of line. */
  code_size = block_height * ((block_width / 2 + 1) * 2 + 2);

  XMALLOC_VAR (sprite);
  XMALLOC_ARRAY (sprite->glenz.code, code_size);

  /* encode the bloc */
  pc = sprite->glenz.code;
  for (row = block_height; row; --row) {
    unsigned int m, n;
    const pixel_t *eol = src + block_width; /* end of line */

    /* encode a line */
    do {
      /* count the number of transparant pixels */
      for (m = 0; *src == transp_color && src < eol && m < 255; ++src)
	++m;
      /* count the number of glenz pixels */
      for (n = 0; *src != transp_color  && src < eol && n < 255; ++src)
	++n;
      /* write the corresponding data */
      *pc++ = m;
      *pc++ = n;
    } while (src < eol);
    /* output an end of line */
    *pc++ = 0;
    *pc++ = 0;

    /* prepare for next line */
    src += src_width - block_width;
  }

  sprite->glenz.kind = S_RLE_GLENZ;
  sprite->glenz.draw = draw_sprglenz;
  sprite->glenz.end_code = pc;
  sprite->glenz.line_skip = dest_width - block_width;
  sprite->glenz.glenz = glenz_line;

  assert (pc < sprite->glenz.code + code_size);

  return sprite;
}

void free_sprglenz (sprite_t *prog)
{
  assert (prog->all.kind == S_RLE_GLENZ);
  free (prog->glenz.code);
  free (prog);
}
