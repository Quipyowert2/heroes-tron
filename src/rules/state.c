/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001, 2002  Alexandre Duret-Lutz      |
|                                          <duret_g@epita.fr>       |
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

#include "prefs.h"		/* FIXME: Get rid of this include. */

void
state_init (a_level_state *state, const a_level *lvl, char cont,
	    bool two_players, bool in_menu)
{
  a_level_state_bits *bits;

  state->level = lvl;

  XSALLOC_ARRAY (state->square_occupied, lvl->square_count, SQOC_VACANT);
  XMALLOC_ARRAY (state->square_way, lvl->square_count);

  /* Init square_coord, map square indices to tile indices.  */
  {
    a_square_coord i, j;
    a_tile_index k = 0;
    a_square_index l = 0, k2;
    XMALLOC_ARRAY (state->square_tile, lvl->square_count);
    for (i = 0, l = 0; i < lvl->tile_height; i++, l += lvl->square_width * 2) {
      for (j = 0, k2 = l; j < lvl->tile_width; j++, k2 += 2, k++) {
	state->square_tile[k2] = k;
	state->square_tile[k2 + 1] = k;
	state->square_tile[lvl->square_width + k2] = k;
	state->square_tile[lvl->square_width + k2 + 1] = k;
      }
    }
  }

  /* Init square_coord, map indices to coordinates.   */
  {
    a_square_coord i, j;
    XCALLOC_ARRAY (state->square_coord, lvl->square_count);
    for (j = 0; j < lvl->square_height; j++)
      for (i = 0; i < lvl->square_width; i++) {
	state->square_coord[SQR_COORDS_TO_INDEX (lvl, j, i)].y = j;
	state->square_coord[SQR_COORDS_TO_INDEX (lvl, j, i)].x = i;
      }
  }

  XCALLOC_VAR (state->private);
  bits = state->private;

  state->square_object = 0;

  if (state->game_mode == M_KILLEM) {
    XCALLOC_ARRAY (state->square_lemmings_list, lvl->square_count);
    XCALLOC_ARRAY (state->square_dead_lemmings_list, lvl->square_count);
    memset (bits->lemmings_support, 0,
	    LEMMINGS_TOTAL * sizeof (*state->private->lemmings_support));
  } else if (state->game_mode >= M_TCASH) {
    XMALLOC_ARRAY (state->square_object, lvl->square_count);
  }

  bits->level_is_finished = 0;

  /* init of players  */
  if (!in_menu) {
    unsigned i;
    for (i = 0; i < 4; ++i) {
      /* trail_offset[i]=0; */
      if (state->game_mode == M_DEATHM) {
	bits->trail_size[i] = 32;
	state->player[i].lifes = 9;
      } else
	bits->trail_size[i] = 5;
      state_reinit_player (state, i);
      if (cont == 0) {
	state->player[i].lifes = 9;
	state->player[i].score = 0;
	state->player[i].wins = 0;
      } else {
	/* reinitialize dead computers: give them an empty score
	   and decrase their total of wins */
	if (state->player[i].cpu < 2 && state->player[i].lifes == 0)
	  {
	    state->player[i].lifes = 9;
	    state->player[i].score = 0;
	    if (state->player[i].wins > 0)
	      --state->player[i].wins;
	  }
      }
      state->player[i].autopilot = 1;
      state->player[i].score_delta = state->player[i].score << 2;
      state->player[i].invincible = 0;
      state->player[i].time = 3000;
      state->player[i].cash = 0;
      state->player[i].martians_nbr = 0;
    }
  } else {
    unsigned i;
    for (i = 0; i < 4; ++i) {
      bits->trail_size[i] = 0;
    }
  }
  /* reinit player once again to avoid the case where
     some vehicles could have been put in front of others */
  state_erase_player (state, 0);
  state_reinit_player (state, 0);
  state_erase_player (state, 1);
  state_reinit_player (state, 1);
  state_erase_player (state, 2);
  state_reinit_player (state, 2);
  state_erase_player (state, 3);
  state_reinit_player (state, 3);

  if (!opt.autopilot_one)
    state->player[state->col2plr[0]].autopilot = 0;
  if (two_players && !opt.autopilot_two)
    state->player[state->col2plr[1]].autopilot = 0;

  if (state->game_mode == M_KILLEM)
    state_init_lemmings (state);

  if (state->game_mode >= M_TCASH) {
    unsigned i;
    for (i = 0; i < lvl->square_count; ++i)
      if (lvl->square_type[i] == T_OUTWAY)
	state->square_object[i] = SQOB_UNREACHABLE;
      else
	state->square_object[i] = SQOB_NOTHING;
    if (state->game_mode == M_COLOR) {
      bits->objects_nbr = lvl->square_count / 14 + 1;
      for (i = bits->objects_nbr; i != 0; i--)
	add_color (state, 1);
    }
    if (state->game_mode == M_TCASH) {
      bits->objects_nbr = lvl->square_count / 13 + 1;
      for (i = bits->objects_nbr; i != 0; i--)
	add_cash (state, 1);
    }
  }

  allocate_explosions (state);
  init_bonuses_level (state);
  if (!in_menu)
    spread_bonuses (state);
}

void
state_free (a_level_state *state)
{
  uninit_bonuses_level (state);
  release_explosions (state);

  free (state->square_occupied);
  free (state->square_way);
  free (state->square_tile);
  free (state->square_coord);

  XFREE (state->square_lemmings_list);
  XFREE (state->square_dead_lemmings_list);
  XFREE (state->square_object);

  free (state->private);
}

void
state_set_player_color (a_level_state *state, unsigned player, unsigned color)
{
  state->col2plr[color] = player;
  state->plr2col[player] = color;
}

int
state_level_exit_code (const a_level_state *state)
{
  return state->private->level_is_finished;
}

void
state_level_set_exit_code (a_level_state *state, int code)
{
  state->private->level_is_finished = code;
}
