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

#ifndef HEROES__STATEPRIV__H
#define HEROES__STATEPRIV__H

/*---------------------------------------------------------------.
| NOTE: This is a private header, for use in the `rules' library |
| only.  Clients of the `rules' module should only rely on the   |
| `state.h' interface.                                           |
`---------------------------------------------------------------*/

/* We don't want to see constant attribute from here.  */
#ifndef LVL_STATE_MUTABLE
# define LVL_STATE_MUTABLE
#endif
#include "state.h"
#include "timer.h"

typedef struct an_explosion_info an_explosion_info;

#define maxq 128
struct a_level_state_bits {
  a_square_index trail_pos[4][maxq];
  a_dir8_pair trail_way[4][maxq];
  unsigned trail_offset[4];
  unsigned trail_size[4];	/* size of trails, minus one */

#define LEMMINGS_TOTAL (LEMMINGS_PER_PLAYERS*4)
  a_lemming lemmings_support[LEMMINGS_TOTAL];
  int objects_nbr;
  int lemmings_move_offset;

  int level_is_finished;

  an_explosion_info *explo_list;
  unsigned int explo_list_max;
  unsigned int explo_list_first_unused;

  a_timer explo_timer;
  long explo_time;		/* Updated from explo_timer on each call
				   to update_explosion.  */

  /* FIXME: tile_bonus_cpu is a temporary array used by ai.c.
     It should NOT be defined here. */
  a_u8 *tile_bonus_cpu;

  a_timer update_timer;
  bool players_started;
};

extern void add_color (a_level_state *state, bool allow_clocks);
extern void add_cash (a_level_state *state, bool allow_clocks);

extern void grow_trail (a_level_state *state, int pl, int size);
extern void shrink_trail (a_level_state *state, int pl, int size);
extern void erase_trail (a_level_state *state, int c);

extern int bonus_points[2][17];	/* interest of bonuses,
				   for CPU controled vehicles */

extern int *bonus_time;
extern int *bonus_list;

extern void add_random_bonus (a_level_state *state, int pos_in_list);
extern void rem_bonus (a_level_state *state, int pos);

/* reset and allocate bonus data for a given level */
extern int init_bonuses_level (a_level_state *state);
extern void uninit_bonuses_level (a_level_state *state);

extern void spread_bonuses (a_level_state *state);

extern void add_end_level_bonuses (a_level_state *state);
extern void apply_bonus (a_level_state *state, int pl, char bonus);

extern void allocate_explosions (a_level_state *state);
extern void release_explosions (a_level_state *state);

/* FRAME_START is expected to be EXPLOSION_IMMEDIATE or EXPLOSION_TRIGGERED. */
extern void trigger_explosion (a_level_state *state,
			a_square_index idx, an_explosion frame_start);
extern void trigger_possible_explosion (a_level_state *state,
					a_square_index idx);
extern void update_explosions (a_level_state *state);

extern void update_bonuses (a_level_state *state);

extern void update_lemmings (a_level_state *state);
extern void update_player (a_level_state *state, unsigned c);

#endif /* HEROES__STATEPRIV__H */
