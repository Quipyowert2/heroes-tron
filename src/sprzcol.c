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
#include "sprzcol.h"

void
draw_sprzcol (const sprite_t *sprite, pixel_t *dest)
{
  pixel_t	*cur = dest;	/* current writting possition */
  u8_t		*pc;		/* program counter */
  u8_t		*epc;		/* end of program code */

  assert (sprite->all.kind == S_RLE_ZCOL);

  pc = sprite->rle.code;
  epc = sprite->rle.end_code;

  while (pc < epc) {
    unsigned m, n;
    m = *pc++;
    n = *pc++;
    if (n == 0 && m == 0) {	/* end of line */
      cur += sprite->rle.line_skip;
    } else {
      cur += m;
      for (; n; --n) {
	pixel_t curcol;
	curcol = *cur;
	if (curcol > 15 || curcol < *pc)
	  *cur = *pc;
	++pc;
	++cur;
      }
    }
  }
}

sprite_t *
compile_sprzcol (const pixel_t *src, pixel_t transp_color,
		 unsigned int block_height, unsigned int block_width,
		 unsigned int src_width, unsigned int dest_width)
{
  sprite_t *tmp = compile_sprrle (src, transp_color, block_height, block_width,
				  src_width, dest_width);
  tmp->all.kind = S_RLE_ZCOL;
  tmp->all.draw = draw_sprzcol;
  return tmp;
}

void
free_sprzcol (sprite_t *prog)
{
  prog->all.kind = S_RLE;
  free_sprrle (prog);
}
