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
#include "sprprog.h"
#include "generic_list.h"

void
draw_sprprog (const sprite_t *sprite, pixel_t *dest)
{
  sprite_prog_list_t *list;

  assert (sprite->all.kind == S_PROG || sprite->all.kind == S_PROG_WAV);

  for (list = sprite->prog.list; list; list = list->cdr) {
    sprite_t *s = list->car;
    s->draw (s, dest + list->offset);
  }
}

void
draw_sprprog_clipped_left (const sprite_t *sprite, pixel_t *dest,
			   int dest_col, int min_col)
{
  sprite_prog_list_t *list;

  assert (sprite->all.kind == S_PROG || sprite->all.kind == S_PROG_WAV);

  for (list = sprite->prog.list; list; list = list->cdr) {
    sprite_t *s = list->car;
    if (list->offset + dest_col > min_col)
      s->draw (s, dest + list->offset);
  }
}

void
draw_sprprog_clipped_right (const sprite_t *sprite, pixel_t *dest,
			    int dest_col, int max_col)
{
  sprite_prog_list_t *list;

  assert (sprite->all.kind == S_PROG || sprite->all.kind == S_PROG_WAV);

  for (list = sprite->prog.list; list; list = list->cdr) {
    sprite_t *s = list->car;
    if (list->offset + dest_col < max_col)
      s->draw (s, dest + list->offset);
  }
}


/* internal state */
static sprite_prog_list_t* prog = 0;
static sprite_prog_list_t** last = 0;

/* we need to save these two state, to allow recursive calls */
NEW_LIST (prog, sprite_prog_list_t*, STD_EQUAL, free);
NEW_LIST (last, sprite_prog_list_t**, STD_EQUAL, free);
prog_list_t prog_save = 0;
last_list_t last_save = 0;

void
new_sprprog (void)
{
  prog_push (&prog_save, prog);
  last_push (&last_save, last);
  last = &prog;
}


void
add_sprprog (sprite_t *sprite, int offset)
{
  XMALLOC_VAR (*last);
  (*last)->car = sprite;
  (*last)->offset = offset;
  last = &((*last)->cdr);
}

void
add_sprprog0 (sprite_t *sprite)
{
  add_sprprog (sprite, 0);
}

sprite_t *
end_sprprog (void)
{
  NEW (sprite_t, res);
  *last = 0;			/* terminate the list */
  res->prog.kind = S_PROG;
  res->prog.draw = draw_sprprog;
  res->prog.list = prog;
  prog = prog_pop (&prog_save);
  last = last_pop (&last_save);
  return res;
}

void
free_sprprog (sprite_t *sprite)
{
  sprite_prog_list_t *list = sprite->prog.list;
  while (list) {
    sprite_prog_list_t *next = list->cdr;
    free_sprite (list->car);
    free (list);
    list = next;
  }
  free (sprite);
}
