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


typedef struct a_lemming a_lemming;
struct a_lemming {
  unsigned int pos_head, pos_tail; /* positions */
  unsigned int min;		/* position in the tile */
  a_lemming *next_dead;		/* next stain in the tile */
  a_dir dir;
  int couleur;
  char dead;
};

#define maxq 128


struct a_level_state_bits {
  a_square_index trail_pos[4][maxq];
  a_dir8_pair trail_way[4][maxq];
  unsigned trail_offset[4];
  unsigned trail_size[4];	/* size of trails, minus one */

#define lemmings_per_players 50
#define lemmings_total (lemmings_per_players*4)
  a_lemming **square_lemmings_list;
  a_lemming **square_dead_lemmings_list;
  a_lemming lemmings_support[lemmings_total];
  int objects_nbr;

  int level_is_finished;
};

void add_color (a_level_state *state, const a_level *lvl, bool allow_clocks);
void add_cash (a_level_state *state, const a_level *lvl, bool allow_clocks);

void grow_trail (a_level_state *state, int pl, int size);
void shrink_trail (a_level_state *state, int pl, int size);
void erase_trail (a_level_state *state, const a_level *lvl, int c);

#endif /* HEROES__STATEPRIV__H */
