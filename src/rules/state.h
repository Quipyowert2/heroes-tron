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
# include "timer.h"
# include "bonuses.h"

/** -- BEGIN PUBLIC -- **/

/* Information about a lemming.  */
typedef struct a_lemming a_lemming;
struct a_lemming {
  /* Position of the lemming.

     Lemmings are always astride on two squares when moving.  If
     pos_head == pos_tail, then the lemmings is stopped.

     When the lemming is moving, the offset within the current square
     is common with all lemmings (they move synchornously) and
     returned by the state_lemmings_move_offset() function.  */
  a_square_index pos_head, pos_tail;

  /* Direction the lemming looks towards.  */
  a_dir dir;

  /* Color of the lemmings.  */
  int color;

  /* True iff the lemming is dead.  */
  bool dead;

  /* For dead lemmings, i.e. blood stains, the position within the
     square is NOT the one given by state_lemmings_move_offset(),
     otherwise the stains would be moving... */
  unsigned int puddle_offset;

  /* Several puddle can be accumulated on the same square.  */
  a_lemming *next_puddle;
};

/* When playing "Kill'em All", each player start with
   LEMMINGS_PER_PLAYERS lemmings.  */
#define LEMMINGS_PER_PLAYERS 50

/* The game modes, as selected in the PLAY menu.  */
typedef enum a_game_mode a_game_mode;
enum a_game_mode {
  M_QUEST = 0,			/* Quest */
  M_DEATHM = 1,			/* Death-match */
  M_KILLEM = 2,			/* Kill'em all */
  M_TCASH = 3,			/* Time-ca$h */
  M_COLOR = 4,			/* Color */
};

/* An explosions is displayed using NBR_EXPLOSION_FRAMES intermediate
   sprites.  There is NBR_EXPLOSION_KINDS different set of explosions
   sprites (the set is chosen randomly before an explosion is displayed,
   mixing them make large flames looks better).
   It should be easy the add new kinds of explosions.  Adding more
   frame however would need several timing adjustement in the code.  */
#define NBR_EXPLOSION_FRAMES 15
#define NBR_EXPLOSION_KINDS   2

/* Number of time slices for each frames.  Changing this affects
   the speed of the explosions.  */
#define EXPLOSION_SLICES_PER_FRAMES 8
/* Number of frames between the moment an explosions is triggered, and
   the moment it actually explodes.  */
#define EXPLOSION_DELAY 8

/*
 *  Various constants, used for the an_explosion type.
 */

/* The explosion will explode immediately.  */
#define EXPLOSION_IMMEDIATE (NBR_EXPLOSION_FRAMES - 1)
/* The explosion is triggered, it will explode later.  */
#define EXPLOSION_TRIGGERED (EXPLOSION_IMMEDIATE + EXPLOSION_DELAY)
/* It's time for the explosion to trigger neighbors.  */
#define EXPLOSION_TRIGGER_NEIGHBORS (EXPLOSION_TRIGGERED - 3)
/* The explosion is not triggered (it's also triggerable).  */
#define EXPLOSION_UNTRIGGERED (EXPLOSION_TRIGGERED + 1)

/*
 * 0 <= n < NBR_EXPLOSION_FRAMES: frame number to display.
 * NBR_EXPLOSION_FRAMES <= n <= EXPLOSION_TRIGGERED: about to explode.
 * n == EXPLOSION_UNTRIGGERED: idle.
 */
typedef a_u8 an_explosion;
#define EXPLOSION_SQUARE_TRIGGERED_P(state_ptr, idx)		\
  ((state_ptr)->square_explo_state[idx] <= EXPLOSION_TRIGGERED)
#define EXPLOSION_SQUARE_TRIGGERABLE_P(state_ptr, idx)		\
  ((state_ptr)->level->square_type[idx] == T_BOOM		\
   && (state_ptr)->square_explo_state[idx] == EXPLOSION_UNTRIGGERED)

/* Some pointers in the a_level_state structure should point to
   constant data, so the user a warned if s/he modify it.  However we
   want to allow the functions from the libhrules.a library to modify
   them. So we allow const to be removed by defining
   LVL_STATE_MUTABLE.  */

#ifndef LVL_STATE_MUTABLE
# define LVL_STATE_MUTABLE const
#endif

/* Dynamic state of a level.  */
typedef struct a_level_state a_level_state;
/* Hidden details in a_level_state */
typedef struct a_level_state_bits a_level_state_bits;

struct a_level_state {
  /* The level definition. (Always const.) */
  const a_level *level;

  /* Informations about each player.  */
  a_player *player[4];		/* FIXME: use LVL_STATE_MUTABLE */

  /* State of each square.
     0xFF: free
     4,5,6,7: heads of the vehicles
     0,1,2,3: tails of the vehicles (and heads of the trails)
     8,9,10,11: trails
     12,13,14,15: tails of the trails.
  */
  LVL_STATE_MUTABLE a_u8 *square_occupied;
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

  /* Objet lying on the level (pyramids, dollars, clocks, ...).
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
  a_s8 *square_object;
#define SQOB_NOTHING (-1)
#define SQOB_UNREACHABLE (-2)
  /* SQOB = SQuare OBject.  */

  a_game_mode game_mode;

  /* The state of the explosion for each square.  See the EXPLOSION_*
     constants above.  */
  LVL_STATE_MUTABLE an_explosion *square_explo_state;
  /* The type of each explosion, smaller than NBR_EXPLOSION_KINDS
     for each square.   This is really usefull only to render
     the level; all explosions have the same effect on the players. */
  LVL_STATE_MUTABLE a_u8 *square_explo_type;

  /* For each tile, the number of the bonus present, if any.  */
  LVL_STATE_MUTABLE a_bonus8 *tile_bonus;

  /* For each square occupied by a lemming, this holds a pointer to
     the lemming data.  If the square is empty, the pointer is NULL.  */
  a_lemming **square_lemmings_list;

  /* For each square occupied by a puddle, this holds a pointer to the
     lemmings data.  Several puddles might be chained with the
     next_puddle fieds of this structure.  The pointer is NULL if
     there is no puddle.  Blood puddles have no effect on the players
     (as far the rules of the game are concerned), so this array
     should be useful only to the renderer.  */
  a_lemming **square_dead_lemmings_list;

  /* Private data.  Use the state_* functions to access them.  */
  LVL_STATE_MUTABLE a_level_state_bits *private;
};

/* Size of the player's trail (i.e. number of L+ bonus eaten + 1).  */
int state_trail_size (const a_level_state *state, int player);

/* Return TRUE if the trail is expending.  */
bool state_trail_expending (const a_level_state *state, int player);

/** -- END PUBLIC -- **/

void state_erase_player (a_level_state *state, unsigned i);
void state_reinit_player (a_level_state *state, unsigned p);
void state_init_lemmings (a_level_state *state);
void state_init (a_level_state *state, const a_level *lvl, char cont,
		 bool two_players, bool in_menu);
void state_free (a_level_state *state);

void state_set_player_color (a_level_state *state,
			     unsigned player, unsigned color);

int state_lemmings_move_offset (a_level_state *state);

int state_level_exit_code (const a_level_state *state);
void state_level_set_exit_code (a_level_state *state, int code);

void state_start_game (a_level_state *state);
int state_update (a_level_state *state);
void state_start_players (a_level_state *state);
void state_pause (a_level_state *state, const a_timer pause_timer);

#endif /* HEROES__STATE__H */
