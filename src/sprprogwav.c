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
#include "sprprogwav.h"

static int
waving_offset (int pos)
{
  return ceil (sin (((text_waving_step + pos*2) & 31) * 3.141592653 / 16.0)
	       * 1.7) * xbuf;
}

void
draw_sprprogwav (const sprite_t *sprite, pixel_t *dest)
{
  sprite_prog_list_t *list;
  int pos = 0;

  assert (sprite->all.kind == S_PROG || sprite->all.kind == S_PROG_WAV);

  for (list = sprite->prog.list; list; list = list->cdr) {
    sprite_t *s = list->car;
    s->draw (s, dest + list->offset + waving_offset (pos++));
  }
}

sprite_t *
end_sprprogwav (void)
{
  sprite_t *s = end_sprprog ();
  s->all.kind = S_PROG_WAV;	/* overwrite */
  return s;
}
