/*------------------------------------------------------------------.
| Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License version 2 as    |
| published by the Free Software Foundation.                        |
|                                                                   |
| Heroes is distributed in the hope that it will be useful, but     |
| WITHOUT ANY WARRANTY; without even the implied warranty of        |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU |
| General Public License for more details.                          |
|                                                                   |
| You should have received a copy of the GNU General Public License |
| along with this program; if not, write to the Free Software       |
| Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          |
| 02111-1307 USA                                                    |
`------------------------------------------------------------------*/

#include "system.h"
#include "statepriv.h"
#include "bonus.h"
#include "explosions.h"

#include "sfx.h"		/* FIXME: Do we want this? */

int state_trail_size (const a_level_state *state, int player)
{
  return (state->private->trail_size[player] + 1) / 5 - 1;
}

void
grow_trail (a_level_state *state, int pl, int size)
{
  int i, k;
  a_level_state_bits *bits = state->private;

  k = ((bits->trail_offset[pl] + bits->trail_size[pl] - 1) & (maxq - 1));
  while (size != 0 && bits->trail_size[pl] + 5 < maxq) {
    i = ((bits->trail_offset[pl] + bits->trail_size[pl]) & (maxq - 1));
    bits->trail_pos[pl][i] = bits->trail_pos[pl][k];
    bits->trail_way[pl][i] = bits->trail_way[pl][k];
    bits->trail_size[pl]++;
    --size;
  }

  if (bits->trail_size[pl] >= 55 && state->game_mode == M_QUEST) {
    if (state->player[pl].cpu == 2)
      event_sfx (89);
    add_end_level_bonuses (state);
  }
}

/* Shrink the trail for player PL by SIZE squares.  */
void
shrink_trail (a_level_state *state, int pl, int size)
{
  int i;
  a_level_state_bits *bits = state->private;

  while (size != 0 && bits->trail_size[pl] > 5) {
    --size;
    --bits->trail_size[pl];
    i = ((bits->trail_offset[pl] + bits->trail_size[pl]) & (maxq - 1));
    state->square_occupied[bits->trail_pos[pl][i]] = SQOC_VACANT;
    i = ((bits->trail_offset[pl] + bits->trail_size[pl] - 1) & (maxq - 1));
    /* Setup the new trail tail, but make sure we don't redraw
       anything if the trail has been erased before (this happens when
       the player dies: the square is erased (it explodes) and then
       the tail is shrinked).  */
    if (state->square_occupied[bits->trail_pos[pl][i]] != SQOC_VACANT)
      state->square_occupied[bits->trail_pos[pl][i]] = SQOC_TRAIL_TAIL (pl);
  }
}

void
erase_trail (a_level_state *state, int c)
{
  a_square_index i;

  for (i = 0; i < state->level->square_count; ++i)
    if ((state->square_occupied[i] & 3) == c
	&& state->square_occupied[i] < 16) {
      state->square_occupied[i] = SQOC_VACANT;
      trigger_explosion (state, i, EXPLOSION_IMMEDIATE);
    }
}

bool
state_trail_expending (const a_level_state *state, int player)
{
  int tmp1 = state->private->trail_offset[player]
    + state->private->trail_size[player] - 1;
  return (state->private->trail_pos[player][tmp1 & (maxq - 1)]
	  == state->private->trail_pos[player][(tmp1 - 1) & (maxq - 1)]);
}
