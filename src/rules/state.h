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

#ifndef HEROES__STATE__H
#define HEROES__STATE__H
# include "lvl.h"
# include "player.h"

/* Dynamic state of a level.  */
typedef struct a_level_state a_level_state;
/* Hidden details in a_level_state */
typedef struct a_level_state_bits a_level_state_bits;

typedef enum a_game_mode a_game_mode;
enum a_game_mode {
  M_QUEST = 0,
  M_DEATHM = 1,
  M_KILLEM = 2,
  M_TCASH = 3,
  M_COLOR = 4,
};

/* Some pointers in the a_level_state structure should point to
   constant data, so the user a warned if s/he modify it.  However we
   want to allow the functions from the libhrules.a library to modify
   them. So we allow const to be removed by defining
   LVL_STATE_MUTABLE.  */

#ifndef LVL_STATE_MUTABLE
# define LVL_STATE_MUTABLE const
#endif


struct a_level_state {
  a_player player[4];		/* Informations about each player.  */

  /* State of each square.
     0xFF: free
     4,5,6,7: heads of the vehicles
     0,1,2,3: tails of the vehicles (and heads of the trails)
     8,9,10,11: trails
     12,13,14,15: tails of the trails.
  */
  LVL_STATE_MUTABLE unsigned char *square_occupied;
#define SQOC_VEHICLE_TAIL(color) (color)
#define SQOC_VEHICLE_HEAD(color) ((color) + 4)
#define SQOC_TRAIL(color) ((color) + 8)
#define SQOC_TRAIL_TAIL(color) ((color) + 12)
#define SQOC_VACANT 0xFFU
  /* SQOC = SQuare OCcupied.  */

  /* FIXME: Document or remove.  */
  int col2plr[4];
  int plr2col[4];

  /* If the square is occupied, this indicate the directions of the
     trail.  */
  LVL_STATE_MUTABLE a_dir8_pair *square_way;

  /* Fast conversion between a_square_index and a_tile_index.  */
  LVL_STATE_MUTABLE a_tile_index *square_tile;

  /* Fast conversion between a_square_index and a_square_coord_pair.  */
  a_square_coord_pair *square_coord;

  /* Objet lying on the levle (pyramids, dollars, clocks, ...).
      -1: nothing
      -2: can't drive here
     In TIME CASH mode:
      0: dollar
      15: clock
     In COLOR mode:
      0,1,2,3,4: color gems (4 = grey)
      8,9,10,11,12: checked color gems
      16: clock
      24: checked clock
   */
  signed char *square_object;
#define SQOB_NOTHING (-1)
#define SQOB_UNREACHABLE (-2)
  /* SQOB = SQuare OBject.  */

  a_game_mode game_mode;

  /* Private data.  Use the state_* functions to access them.  */
  LVL_STATE_MUTABLE a_level_state_bits *private;
};

#include "statepriv.h"		/* FIXME: Get rid of this.  */

/* Size of the player's trail (i.e. number of L+ bonus eaten + 1).  */
int state_trail_size (const a_level_state *state, int player);

void state_erase_player (a_level_state *state, const a_level *lvl, unsigned i);
void state_reinit_player (a_level_state *state, const a_level *lvl,
			  unsigned p);
void state_init_lemmings (a_level_state *state, const a_level *lvl);
void state_init (a_level_state *state, const a_level *lvl, char cont);
void state_free (a_level_state *state);

void state_set_player_color (a_level_state *state,
			     unsigned player, unsigned color);

void update_lemmings (a_level_state *state, const a_level *lvl);

#endif /* HEROES__STATE__H */
