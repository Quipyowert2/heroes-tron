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

static void
find_lemming_direction (a_level_state *state, const a_level *lvl,
			a_lemming *lem)
{
  a_dir d;
  a_dir_mask avail_dirm;

  /* The lemming advances one square.  The tail takes the place of
     the head.  */
  state->square_lemmings_list[lem->pos_tail] = NULL;
  lem->pos_tail = lem->pos_head;
  state->square_lemmings_list[lem->pos_tail] = lem;

  /* We need to compute the new square for the head (i.e. the
     direction of the lemming).  */

  /* Mask AVAIL_DIRM with all the directions impossible to use
     by the lemming.  */
  avail_dirm = DM_ALL;
  for (d = 0; d < DIR_MAX; ++d) {
    a_square_index dest = lvl->square_move[d][lem->pos_tail];

    if (/* Lemmings can't cross walls, */
	dest == INVALID_INDEX
	/* they should not go to a square occupied by a player, */
	|| state->square_occupied[dest] != SQOC_VACANT
	/* neither should they go to a quare occupied by another
	   lemming.  */
	|| state->square_lemmings_list[dest])
      avail_dirm &= ~DIR_TO_DIRMASK(d);
  }
  /* Also, we don't want to allow lemmings to enter
     tunnels.  This constraint might be relaxed in the
     future, but actually this requires several things that
     I (adl) don't plan to work on at the moment:
      - handling two directions for each lemming: for the
        head and the tail, because the direction at the output
        of a tunnel might not be the same as the entrance
      - drawing the lemmings in two parts (since they need
        to be cropped under the tunnel entrance)
      - handling dead lemmings (blood pudles) on two
        non-adjacent squares (e.g. the entrance and the output
        of the tunnel, in case the lemming has been squished in
        a tunnel).
     This latter point is not only a rendering issue: the
     way dead lemmings are stored needs to be changed too
     (at the time this comment is written, they are assigned
     to *the* nearest square, and linked to the other dead
     lemmings of this square).

     So, for now, let's just prevent them from turning toward
     a tunnel entrance.  */
  if (lvl->square_type[lem->pos_tail] == T_TUNNEL)
    avail_dirm &= ~DIR_TO_DIRMASK (lvl->square_direction[lem->pos_tail]);

  /* Is the current direction available?  (We'd better avoid changing
     the direction on each square unless we decide lemmings are bees.)
     */
  if (avail_dirm & DIR_TO_DIRMASK(lem->dir)) {
    /* The current direction is free.  Go on.  */
    lem->pos_head = lvl->square_move[lem->dir][lem->pos_tail];
  } else {
    /* Current direction unavalaible.  Let's find another one.  */
    a_dir_mask i;
    int n = 0;

    /* Count the number of direction available.  */
    for (i = 1; i < DM_ALL; i <<= 1)
      if (avail_dirm & i)
	++n;

    if (n) {
      /* Chose one of the free directions. */
      n = 1 + rand () % n;
      for (d = 0; n != 0; ++d, avail_dirm >>= 1)
	if (avail_dirm & 1)
	  --n;
      lem->dir = d - 1;
      lem->pos_head = lvl->square_move[lem->dir][lem->pos_tail];
    } else {
      /* This place is reached when there is no free way for the
	 lemming to use.  We used to set lem->dir = 5; to indicate
	 this condition to the renderer (when then the lemmings cannot
	 move it should be not be rendered as `walking').  But it
	 turns out that checking whether ptr->pos_tail ==
	 lem->pos_head is sufficient to detect this condition.  */
    }
  }

  assert (lem->pos_head != INVALID_INDEX);
  if (lem->pos_head != lem->pos_tail) {
    /* We assert this only here, because it may happens that a
       lemmings is blocked under a player trail.  */
    assert (state->square_occupied[lem->pos_head] == 0xff);

    /* If the lemming is moving, mark the destination square as
       occupied so that no other lemming dares to move there too. */
    assert (state->square_lemmings_list[lem->pos_head] == 0);
    state->square_lemmings_list[lem->pos_head] = lem;
  }
}

void
state_init_lemmings (a_level_state *state, const a_level *lvl)
{
  a_lemming *ptir = state->private->lemmings_support;
  unsigned i, j;
  for (i = 0; i < 4; i++) {
    for (j = lemmings_per_players; j != 0; j--) {
      /* Drop the lemming randomly on an accessible unoccupied
	 square.  The lemmings might still not be able to move,
	 but it's unimportant: it should just stay still until
	 it can move.  */
      a_square_index k;
      do {
	k = rand () % lvl->square_count;
	assert (k < lvl->square_count);
      } while (lvl->square_type[k] == T_OUTWAY
	       || state->square_occupied[k] != SQOC_VACANT
	       || state->square_lemmings_list[k] != NULL);
      ptir->pos_head = k;
      find_lemming_direction (state, lvl, ptir);
      ptir->min = 0;
      ptir->next_dead = NULL;
      ptir->couleur = i;
      ptir->dead = 0;
      ptir++;
    }
    state->player[i].lemmings_nbr = lemmings_per_players;
  }
  assert (ptir == state->private->lemmings_support + 4 * lemmings_per_players);
  state->private->lemmings_move_offset = 0;
}

void
update_lemmings (a_level_state *state, const a_level *lvl)
{
  a_level_state_bits *bits = state->private;

  bits->lemmings_move_offset += 1024;

  /* Update lemmings directions each time they get to another square.  */
  if (bits->lemmings_move_offset >= 0xffff) {
    int j;
    a_lemming *lem;

    lem = bits->lemmings_support;
    state->private->lemmings_move_offset &= 0xffff;
    for (j = lemmings_total; j != 0; j--, lem++)
      if (lem->dead == 0) {
	find_lemming_direction (state, lvl, lem);
      }
  }
}

int
state_lemmings_move_offset (a_level_state *state)
{
  return state->private->lemmings_move_offset;
}
