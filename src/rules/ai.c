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

/* FIXME: Move this file elsewhere, in a separate directory.  */

#include "system.h"
#include "statepriv.h"
#include "ai.h"
#include "hooks.h"

#include "explosions.h"		/* FIXME: Get rid of this.  */
#include "bonus.h"		/* FIXME: Get rid of this.  */

char ia_max_depth;
char ia_cur_depth;
char ia_is_invincible;
char ia_player;
a_square_coord ia_target_x, ia_target_y;
a_square_coord ia_wrap_x, ia_wrap_y;
char ia_wrap_left, ia_wrap_right;

a_u8 *tile_bonus_cpu = 0;

/* interest of each bonus, for the CPU controled vehicles */
int bonus_points[2][17] =
{ {20, -15, 15, -10, 5, 18, 19, -10, 0, 0, -5, 50, 5, 0, 8, 0, 25}, /* orchid */
  {-15, 10, 0, 10, 5, -5, 0, 8, 8, -5, 5, -20, -5, 9, -10, 9, -25} /* peach */
};


void
ai_level_initialize (a_level_state *state)
{
  XCALLOC_ARRAY (tile_bonus_cpu, state->level->tile_count);
}

void
ai_level_finalize (a_level_state *state)
{
  (void) state;
  XFREE0 (tile_bonus_cpu);
}

/* Adjust speed of AI-player C.  */
void
ai_throttle (a_level_state *state, const a_level *lvl, int c)

{
  /* DEAD_END[C] is TRUE if player C stopped (or tried to) because
     some opponent prevents it to move.  */
  static bool dead_end[4] = {false, false, false, false};

  unsigned free_directions = 4;
  bool seen_opponent = false;
  bool seen_opponent_head = false;
  /* Check squares neighboring next position.  */
  a_square_index next_pos =
    lvl->square_move[state->player[c].way][state->player[c].pos];
  a_dir i;
  for (i = 0; i < 4; i++) {
    a_square_index idx = lvl->square_move[i][next_pos];
    if (idx == INVALID_INDEX || i == REVERSE_DIR (state->player[c].way)) {
      --free_directions;
    } else {
      a_u8 o = state->square_occupied[idx];
      if (o != 0xff) {
	if ((o & 3) != c) {
	  seen_opponent = true;
	  if (o < 8)
	    seen_opponent_head = true;
	}
	--free_directions;
      } else if (state->square_explo_state[idx] < EXPLOSION_IMMEDIATE + 2
		 && ! state->player[c].invincible) {
	--free_directions;
      }
    }
  }

  if (! seen_opponent || free_directions > 1) {
    /* If no opponent is wandering around, or there is at least
       two free direction, there is no need to worry.  */
    state->player[c].turbo = 1;
    dead_end[c] = false;
    return;
  }

  if (free_directions) {
    /* There are some opponent in the vicinity, but there is at least
       one free direction.

       Speed up if we are near an opponent head (maybe he is trying to
       overtake us).  Don't accelerate otherwise, because we would not
       have the time to stop if he cuts us up.  */
    if (seen_opponent_head)
      state->player[c].turbo = 2;
    else
      state->player[c].turbo = 1;
    dead_end[c] = false;
    return;
  }

  if (! dead_end[c]) {
    /* This is a cul de sac and we haven't yet stopped.
       Better do it now.  */
    int chance = 3;
    assert (seen_opponent && free_directions == 0);
    if (state->player[c].rotozoom)
      --chance;
    if (state->player[c].waves)
      --chance;
    if (rand() % 4 <= chance)
      state->player[c].turbo = 0;
    /* Remember we already tried to stop, so we don't give
       another chance to this AI vehicle if it failed to stop.  */
    dead_end[c] = true;
  }
}

static unsigned int
ia_eval_dist (a_level_state *state, const a_level *lvl, int pos)
{
  a_square_coord curx, cury, distx, disty;
  curx = state->square_coord[pos].x;
  cury = state->square_coord[pos].y;
  if (ia_wrap_left) {
    if (curx <= ia_wrap_x)
      distx = curx + lvl->square_width - ia_target_x;
    else if (curx <= ia_target_x)
      distx = ia_target_x - curx;
    else
      distx = curx - ia_target_x;
  } else {
    if (curx >= ia_wrap_x)
      distx = ia_target_x + lvl->square_width - curx;
    else if (curx <= ia_target_x)
      distx = ia_target_x - curx;
    else
      distx = curx - ia_target_x;
  }
  if (ia_wrap_right) {
    if (cury <= ia_wrap_y)
      disty = cury + lvl->square_height - ia_target_y;
    else if (cury <= ia_target_y)
      disty = ia_target_y - cury;
    else
      disty = cury - ia_target_y;
  } else {
    if (cury >= ia_wrap_y)
      disty = ia_target_y + lvl->square_height - cury;
    else if (cury <= ia_target_y)
      disty = ia_target_y - cury;
    else
      disty = cury - ia_target_y;
  }
  return (distx + disty);
}

/* used by ia_goto_target
 */

#define ia_eval_dir_target_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	    tmp = ia_eval_dir_target (state, lvl, idx);			\
	    if (tmp < mindist) mindist = tmp;				\
    }

#define ia_eval_dir_bonus_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
            tmp = ia_eval_dir_bonus (state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ia_eval_dir_lemming_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	    tmp = ia_eval_dir_lemming (state, lvl, idx);		\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ia_eval_dir_cash_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	    tmp = ia_eval_dir_cash (state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ia_eval_dir_color_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	    tmp = ia_eval_dir_color(state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

/*
-> maluses in the position evaluation function
neighb wall ..... : 2pts,
neighb enemy .... : 5pts,
self (old) ...... : 2pts,
self (new) (128). : 1pts,
4pts count as one square in the distance function
*/

static int
ia_eval_neighb_pos (a_level_state *state, const a_level *lvl,
		    a_dir dir, a_square_index pos)
{
  a_square_index idx;
  unsigned char c;
  idx = lvl->square_move[dir][pos];
  if (idx != INVALID_INDEX) {
    c = state->square_occupied[idx];
    if (c < 128) {
      if ((c & 3) == ia_player)
	return 2;
      else
	return 5;
    } else if (c == 128)
      return 1;
    else
      return 0;
  }
  return 2;
}

static unsigned int
ia_eval_dir_target (a_level_state *state, const a_level *lvl,
		    a_square_index pos)
{
  a_u32 mindist;
  a_square_index idx;
  unsigned int tmp;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    state->square_occupied[pos] = 128;
    mindist = U32_MAX;

    ia_eval_dir_target_inline (D_UP);
    ia_eval_dir_target_inline (D_RIGHT);
    ia_eval_dir_target_inline (D_DOWN);
    ia_eval_dir_target_inline (D_LEFT);

    state->square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return mindist;
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (state, lvl, D_UP, pos)
      + ia_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ia_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ia_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return ((ia_eval_dist (state, lvl, pos) << 16)
	    + (tmp << 14) + ia_max_depth - ia_cur_depth);
  }

}

static int
ia_eval_dir_lemming (a_level_state *state, const a_level *lvl,
		     a_square_index pos)
{
  int mindist;
  a_square_index idx;
  int tmp, tmp2;
  a_lemming *tmppti;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    ((signed char*)state->square_occupied)[pos] = 128;
    mindist = 0;
    tmppti = state->square_lemmings_list[pos];
    if (tmppti >= state->private->lemmings_support
	&& tmppti < (state->private->lemmings_support + LEMMINGS_TOTAL)) {
      if (tmppti->color == ia_player)
	tmp2 = -100;
      else
	tmp2 = 20;
    } else
      tmp2 = 0;

    ia_eval_dir_lemming_inline (D_UP);
    ia_eval_dir_lemming_inline (D_RIGHT);
    ia_eval_dir_lemming_inline (D_DOWN);
    ia_eval_dir_lemming_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    ((signed char*)state->square_occupied)[pos] = SQOC_VACANT;
    ia_cur_depth++;
    return mindist;
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (state, lvl, D_UP, pos)
      + ia_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ia_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ia_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }

}

static int
ia_eval_dir_color (a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    ((signed char*)state->square_occupied)[pos] = 128;
    mindist = 0;
    d = state->square_object[pos];
    tmp2 = 0;
    if (d >= 0) {
      if (d <= 4) {
	if (d == ia_player)
	  tmp2 = 100;
	else
	  tmp2 = -200;
      } else if (d <= 12) {
	if (d == ia_player + 8)
	  tmp2 = -200;
	else
	  tmp2 = 100;
      } else if (d == 16)
	tmp2 = 100;
      else if (d == 24)
	tmp2 = -40;
    }

    ia_eval_dir_color_inline (D_UP);
    ia_eval_dir_color_inline (D_RIGHT);
    ia_eval_dir_color_inline (D_DOWN);
    ia_eval_dir_color_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    ((signed char*)state->square_occupied)[pos] = SQOC_VACANT;
    ia_cur_depth++;
    return mindist;
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (state, lvl, D_UP, pos)
      + ia_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ia_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ia_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }
}

static int
ia_eval_dir_cash (a_level_state *state, const a_level *lvl,
		  a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    ((signed char*)state->square_occupied)[pos] = 128;
    mindist = 0;
    d = state->square_object[pos];

    if (d >= 0)
      tmp2 = 500;
    else
      tmp2 = -20;

    ia_eval_dir_cash_inline (D_UP);
    ia_eval_dir_cash_inline (D_RIGHT);
    ia_eval_dir_cash_inline (D_DOWN);
    ia_eval_dir_cash_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    ((signed char*)state->square_occupied)[pos] = SQOC_VACANT;
    ia_cur_depth++;
    return mindist;
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (state, lvl, D_UP, pos)
      + ia_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ia_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ia_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }

}

static int
ia_eval_dir_bonus (a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    int tmp2 = 0;
    int mindist = 0;
    a_tile_index d = state->square_tile[pos];
    a_square_index idx;
    int tmp;
    state->square_occupied[pos] = 128;
    if (tile_bonus_cpu[d] == 0) {
      tmp = state->tile_bonus[d];
      if ((tmp != 0) && (tmp != 0xff)) {
	tile_bonus_cpu[d] = 1;
	if (tmp < 128)
	  tmp2 = bonus_points[0][tmp - 1];
	else
	  tmp2 = bonus_points[1][tmp - 129];
      }
    }
    ia_eval_dir_bonus_inline (D_UP);
    ia_eval_dir_bonus_inline (D_RIGHT);
    ia_eval_dir_bonus_inline (D_DOWN);
    ia_eval_dir_bonus_inline (D_LEFT);

    mindist += tmp2 * (5 + ia_cur_depth) /* /ia_max_depth */ ;

    if (tmp2)
      tile_bonus_cpu[state->square_tile[pos]] = 0;
    state->square_occupied[pos] = SQOC_VACANT;
    ia_cur_depth++;
    return mindist;
  } else {
    int tmp;
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (state, lvl, D_UP, pos)
      + ia_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ia_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ia_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 3);
  }

}

/* give the *way* to follow to get a given position */

#define ia_goto_target_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	  ia_cur_depth=ia_max_depth;					\
	  tmp[dir] = ia_eval_dir_target (state, lvl, idx);		\
	  if (tmp[dir] < mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ia_goto_bonus_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	  ia_cur_depth = ia_max_depth;					\
	  tmp[dir] = ia_eval_dir_bonus(state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ia_goto_lemming_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	  ia_cur_depth = ia_max_depth;					\
	  tmp[dir] = ia_eval_dir_lemming(state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ia_goto_color_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	  ia_cur_depth = ia_max_depth;					\
	  tmp[dir] = ia_eval_dir_color (state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ia_goto_cash_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ia_is_invincible)) {						\
	  ia_cur_depth = ia_max_depth;					\
	  tmp[dir] = ia_eval_dir_cash(state, lvl, idx);			\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

char
ia_goto_target (a_level_state *state, const a_level *lvl,
		int c, int targetx_, int targety_)
{
  a_square_index idx, pos;
  a_u32 tmp[4] = { U32_MAX, U32_MAX, U32_MAX, U32_MAX };
  a_u32 mindist = U32_MAX;
  a_dir mindir = 0;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_target_x = targetx_;
  ia_target_y = targety_;
  ia_wrap_x = ia_target_x + lvl->tile_width;
  if (ia_wrap_x >= lvl->square_width) {
    ia_wrap_x -= lvl->square_width;
    ia_wrap_left = 1;
  } else
    ia_wrap_left = 0;
  ia_wrap_y = ia_target_y + lvl->tile_height;
  if (ia_wrap_y >= lvl->square_height) {
    ia_wrap_y -= lvl->square_height;
    ia_wrap_right = 1;
  } else
    ia_wrap_right = 0;

  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_target_inline (D_UP);
  ia_goto_target_inline (D_RIGHT);
  ia_goto_target_inline (D_DOWN);
  ia_goto_target_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    return state->player[c].way;
  else
    return mindir;
}

char
ia_goto_nearest_bonus (a_level_state *state, const a_level *lvl,
		       int c)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_bonus_inline (D_UP);
  ia_goto_bonus_inline (D_RIGHT);
  ia_goto_bonus_inline (D_DOWN);
  ia_goto_bonus_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    return state->player[c].way;
  else
    return mindir;
}

char
ia_goto_nearest_lemming (a_level_state *state, const a_level *lvl, int c)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_lemming_inline (D_UP);
  ia_goto_lemming_inline (D_RIGHT);
  ia_goto_lemming_inline (D_DOWN);
  ia_goto_lemming_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    return state->player[c].way;
  else
    return mindir;
}

char
ia_goto_nearest_color (a_level_state *state, const a_level *lvl, int c)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_color_inline (D_UP);
  ia_goto_color_inline (D_RIGHT);
  ia_goto_color_inline (D_DOWN);
  ia_goto_color_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    return state->player[c].way;
  else
    return mindir;
}

char
ia_goto_nearest_cash (a_level_state *state, const a_level *lvl, int c)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_cash_inline (D_UP);
  ia_goto_cash_inline (D_RIGHT);
  ia_goto_cash_inline (D_DOWN);
  ia_goto_cash_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    return state->player[c].way;
  else
    return mindir;
}
