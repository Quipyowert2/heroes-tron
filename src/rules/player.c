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
#include "debugmsg.h"
#include "opponents.h"

#include "prefs.h"		/* FIXME: Get rid of this include. */

void
state_erase_player (a_level_state *state, unsigned i)
{
  const a_level *lvl = state->level;
  an_opponent_sig *s = state->private->opponent[i];
  const a_player *p = state->player[i];

  dmsg (D_MISC, "erase player %d", i);

  if (s && s->stop_player)
    state->private->opponent_data[i] =
      s->stop_player(state, i, state->private->opponent_data[i]);

  /* FIXME: this first line used to be guarded by
     if (!in_menu)
     It's not clear to me why this is needed.
  */
  state->square_occupied
    [SQR_COORDS_TO_INDEX (lvl, p->sy, p->sx)] = SQOC_VACANT;

  switch (p->way) {
  case D_LEFT:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, p->sy, SQR_COORD_LEFT (lvl, p->sx))]
      = SQOC_VACANT;
    break;
  case D_RIGHT:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, p->sy, SQR_COORD_RIGHT (lvl, p->sx))]
      = SQOC_VACANT;
    break;
  case D_UP:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, SQR_COORD_UP (lvl, p->sy), p->sx)]
      = SQOC_VACANT;
  case D_DOWN:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, SQR_COORD_DOWN (lvl, p->sy), p->sx)]
      = SQOC_VACANT;
    break;
  default:
    assert (0);
  }
}

static void
state_position_player (a_level_state *state, unsigned i)
{
  const a_level *lvl = state->level;
  const a_player *p = state->player[i];
  dmsg (D_MISC, "position player %d", i);

  /* FIXME: this first line used to be guarded by
     if (!in_menu)
     It's not clear to me why this is needed.
  */
  state->square_occupied
    [SQR_COORDS_TO_INDEX (lvl, p->sy, p->sx)] = SQOC_VEHICLE_TAIL (i);

  switch (p->way) {
  case D_LEFT:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, p->sy, SQR_COORD_LEFT (lvl, p->sx))]
      = SQOC_VEHICLE_HEAD (i);
    break;
  case D_RIGHT:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, p->sy, SQR_COORD_RIGHT (lvl, p->sx))]
      = SQOC_VEHICLE_HEAD (i);
    break;
  case D_UP:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, SQR_COORD_UP (lvl, p->sy), p->sx)]
      = SQOC_VEHICLE_HEAD (i);
    break;
  case D_DOWN:
    state->square_occupied
      [SQR_COORDS_TO_INDEX (lvl, SQR_COORD_DOWN (lvl, p->sy), p->sx)]
      = SQOC_VEHICLE_HEAD (i);
    break;
  default:
    assert (0);
  }
}

void
state_reinit_player (a_level_state *state, unsigned p)
{
  const a_level *lvl = state->level;
  int tries, m;

  unsigned start_pos;		/* 0..3: one of the 4 starting positions.  */
  a_square_coord_pair start_coord;
  a_dir start_dir;
  a_square_index start_idx, next_idx;

  a_level_state_bits *bits = state->private;
  a_player *pp = state->player[p];
  a_player_internal *ipp = &bits->iplayer[p];

  dmsg (D_MISC, "initialize player %d", p);

  tries = 4;
  start_pos = p - 1;
  do {
    --tries;
    start_pos = (start_pos + 1) & 3;
    lvl_start_position (lvl, start_pos, &start_coord, &start_dir);
    start_idx = SQR_COORDS_TO_INDEX (lvl, start_coord.y, start_coord.x);
    next_idx = lvl->square_move[start_dir][start_idx];
  } while (tries && (next_idx == INVALID_INDEX ||
		     state->square_occupied[next_idx] != 0xff));

  pp->way = start_dir;
  pp->sx = start_coord.x;
  pp->sy = start_coord.y;
  pp->si = start_idx;

  /* ensure that the start position is usable,
     otherwise try another position (randomly) */

  for (;;) {
    /* Make sure the selected square is usable (nobody already present,
       and not out of the way).  */
    if (lvl->square_type[start_idx] != T_OUTWAY
	&& state->square_occupied[start_idx] == SQOC_VACANT) {

      a_square_index si;
      bool dir_unusable[4] = { false, false, false, false };

      /* Check directions which are not usable. */

      if ((si = lvl->square_move[D_UP][start_idx]) == INVALID_INDEX
	  || state->square_occupied[si] != SQOC_VACANT)
	dir_unusable[D_UP] = true;
      if ((si = lvl->square_move[D_RIGHT][start_idx]) == INVALID_INDEX
	  || state->square_occupied[si] != SQOC_VACANT)
	dir_unusable[D_RIGHT] = true;
      if ((si = lvl->square_move[D_DOWN][start_idx]) == INVALID_INDEX
	  || state->square_occupied[si] != SQOC_VACANT)
	dir_unusable[D_DOWN] = true;
      if ((si = lvl->square_move[D_LEFT][start_idx]) == INVALID_INDEX
	  || state->square_occupied[si] != SQOC_VACANT)
	dir_unusable[D_LEFT] = true;

      /* is at least one direction ok? */
      if (!(dir_unusable[D_UP]
	    & dir_unusable[D_RIGHT]
	    & dir_unusable[D_DOWN]
	    & dir_unusable[D_LEFT])) {
	/* then rotate until we find that direction */
	while (dir_unusable[pp->way])
	  pp->way = (pp->way + 1) & 3;
	break;
      }
    }
    /* else, get a new position randomly on the map */
    pp->sy = rand () % lvl->square_height;
    pp->sx = rand () % lvl->square_width;
    start_idx = SQR_COORDS_TO_INDEX (lvl, pp->sy, pp->sx);
  }

  pp->tx = pp->sx >> 1;
  pp->ty = pp->sy >> 1;

  /**************/

  pp->ia_max_depth = (rand () & 1) + 5;
  /* Attach the CPU to one of the human player.  */
  {
    int human_players = 0;
    int i;
    for (i = 0; i < 4; ++i)
      if (pp->cpu & 2)
	++human_players;
    /* If there is no human players, attach to any player.  */
    if (human_players == 0)
      human_players = 4;
    pp->target = rand () % human_players;
  }

/* AI won't be enabled on the first moves */
#define ia_skip_firsts_moves 2
  pp->target |= (ia_skip_firsts_moves * 16);

  pp->d.e = 0;
  pp->turbo = 1;
  pp->turbo_level_delta = pp->turbo_level = 1024;
  pp->vitp = ipp->v = 4369 * 3 / 2 + (opt.speed * 2 * 1092);
  pp->spec = 0;
  pp->delay = 0;
  pp->inversed_controls = 0;
  pp->speedup = 0;
  pp->rotozoom = 0;
  pp->waves = 0;
  pp->waves_begin = 0;
  pp->fire_trail = 0;
  pp->tunnel_inverse = 0;
  pp->next_way = ipp->old_old_way = pp->old_way = pp->way;

  state_position_player (state, p);

  start_dir = pp->way;

  state->square_way[start_idx] = DIR8_PAIR (start_dir, start_dir);
  bits->trail_offset[p] = 0;
  for (m = bits->trail_size[p]; m >= 0; m--) {
    bits->trail_pos[p][m] = start_idx;
    bits->trail_way[p][m] = DIR8_PAIR (start_dir, start_dir);
  }

  {
    an_opponent_sig *s = bits->opponent[p];
    if (s && s->start_player)
      bits->opponent_data[p] =
	s->start_player(state, p, bits->opponent_data[p]);
  }
}
