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
#include "explosions.h"
#include "sprzcol.h"
#include "const.h"
#include "timer.h"
#include "debugmsg.h"

a_sprite *explosions[NBR_EXPLOSION_KINDS][NBR_EXPLOSION_FRAMES];

void
init_explosions (void)
{
  int i;
  for (i = 0; i < 6; ++i)
    explosions[0][i] = compile_sprzcol (IMGPOS (vehicles_img, 65, (5 - i)*33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (; i < NBR_EXPLOSION_FRAMES; ++i)
    explosions[0][i] = compile_sprzcol (IMGPOS (vehicles_img, 32,
						(NBR_EXPLOSION_FRAMES - i - 1)
						* 33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (i = 0; i < 6; ++i)
    explosions[1][i] = compile_sprzcol (IMGPOS (vehicles_img, 131, (5 - i)*33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (; i < NBR_EXPLOSION_FRAMES; ++i)
    explosions[1][i] = compile_sprzcol (IMGPOS (vehicles_img, 98,
						(NBR_EXPLOSION_FRAMES - i - 1)
						* 33),
					0, 32, 32, vehicles_img.width, xbuf);
}

void
uninit_explosions (void)
{
  int i, j;
  for (i = 0; i < NBR_EXPLOSION_KINDS; ++i)
    for (j = 0; j < NBR_EXPLOSION_FRAMES; ++j)
      FREE_SPRITE0 (explosions[i][j]);
}

an_explosion *square_explo_state;
a_u8 *square_explo_type;

typedef struct an_explosion_info an_explosion_info;
struct an_explosion_info {
  long orig_time;
  long frame_start;
  a_square_index idx;
  bool neighb_done;		/* True if the explosion has propagated
				   to neighbors.  */
};

static an_explosion_info *explo_list;
static unsigned int explo_list_max;
static unsigned int explo_list_first_unused;

static a_timer explo_timer;
static long explo_time;		/* Updated from explo_timer on each call
				   to update_explosion.  */

void
allocate_explosions (void)
{
  XSALLOC_ARRAY (square_explo_state, lvl.square_count, EXPLOSION_UNTRIGGERED);
  XMALLOC_ARRAY (square_explo_type, lvl.square_count);
  explo_list_max = 64;
  explo_list_first_unused = 0;
  XMALLOC_ARRAY (explo_list, explo_list_max);
  explo_timer = new_htimer (T_GLOBAL, HZ (70));
}

void
release_explosions (void)
{
  free_htimer (explo_timer);
  XFREE0 (square_explo_state);
  XFREE0 (square_explo_type);
  XFREE0 (explo_list);
}

void
trigger_explosion (a_square_index idx, unsigned int frame_start)
{
  if (explo_list_max <= explo_list_first_unused) {
    explo_list_max += 64;
    XREALLOC_ARRAY (explo_list, explo_list_max);
  }
  explo_list[explo_list_first_unused].orig_time = explo_time;
  explo_list[explo_list_first_unused].frame_start = frame_start;
  explo_list[explo_list_first_unused].idx = idx;
  explo_list[explo_list_first_unused].neighb_done = false;
  ++explo_list_first_unused;

  square_explo_type[idx] = rand () % NBR_EXPLOSION_KINDS;
}

void
trigger_possible_explosion (a_square_index idx)
{
  if (lvl.square_type[idx] == T_BOOM
      && square_explo_state[idx] == EXPLOSION_UNTRIGGERED)
    trigger_explosion (idx, EXPLOSION_TRIGGERED);
}

void
update_explosions (void)
{
  unsigned int i, min;

  explo_time = read_htimer (explo_timer);

  if (explo_list_first_unused)
    dmsg (D_MISC, "explosions: %d/%d",
	  explo_list_first_unused, explo_list_max);

  for (min = i = 0; i < explo_list_first_unused; ++i) {
    long duration = explo_time - explo_list[i].orig_time;
    int state =
      explo_list[i].frame_start - duration / EXPLOSION_SLICES_PER_FRAMES;
    if (state >= 0) {
      square_explo_state[explo_list[i].idx] = state;
      /* Trigger neighbor squares if needed.  */
      if (state <= EXPLOSION_TRIGGER_NEIGHBORS && !explo_list[i].neighb_done) {
	a_dir d;
	for (d = 0; d < 4; ++d) {
	  a_square_index ngb = lvl.square_move[d][explo_list[i].idx];
	  if (ngb != INVALID_INDEX)
	    trigger_possible_explosion (ngb);
	}
	explo_list[i].neighb_done = true;
      }
    } else {
      square_explo_state[explo_list[i].idx] = EXPLOSION_UNTRIGGERED;
      /* Count the number of finished explosition at the beginning of
	 the array for removal.  Note that even if EXPLO_LIST is
	 filled in chronologically order, all finished explosions are
	 not necessary grouped at the beginning: they don't all have
	 the same duration.  Here we'll just remove those of them
	 which are at the beginning of the array.  The others will
	 have to wait a bit longuer, that should not be an issue.  */
      if (min == i)
	++min;
    }
  }

  /* Shift the list of explision, removing the finished ones.  */
  explo_list_first_unused -= min;
  memmove (explo_list, explo_list + min,
	   explo_list_first_unused * sizeof (an_explosion_info));
}
