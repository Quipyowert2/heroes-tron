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

#include "plugin.h"

char ai_max_depth;
char ai_cur_depth;
char ai_is_invincible;
char ai_player;
a_square_coord ai_target_x, ai_target_y;
a_square_coord ai_wrap_x, ai_wrap_y;
char ai_wrap_left, ai_wrap_right;

a_u8 *tile_bonus_cpu = 0;
bool *square_marks = 0;		/* Mark the squares inspected while recursing
				   around the current position.  */

/* interest of each bonus, for the CPU controled vehicles */
int bonus_points[2][17] =
{ {20, -15, 15, -10, 5, 18, 19, -10, 0, 0, -5, 50, 5, 0, 8, 0, 25}, /* orchid */
  {-15, 10, 0, 10, 5, -5, 0, 8, 8, -5, 5, -20, -5, 9, -10, 9, -25} /* peach */
};


static void *
ai_level_initialize (const a_level_state *state, int player)
{
  (void) player;
  if (tile_bonus_cpu == 0)
    XCALLOC_ARRAY (tile_bonus_cpu, state->level->tile_count);
  if (square_marks == 0)
    XCALLOC_ARRAY (square_marks, state->level->square_count);
  return 0;
}

static void
ai_level_finalize (const a_level_state *state, int player, void *callback_data)
{
  (void) callback_data;
  (void) state;
  (void) player;
  XFREE0 (square_marks);
  XFREE0 (tile_bonus_cpu);
}

/* Adjust speed of AI-player C.  */
static void
ai_throttle (const a_level_state *state, int p,
	     an_opponent_action *action,
	     void *callback_data)
{
  /* DEAD_END[C] is TRUE if player C stopped (or tried to) because
     some opponent prevents it to move.  */
  static bool dead_end[4] = {false, false, false, false};

  const a_player *const pp = state->player[p];

  unsigned free_directions = 4;
  bool seen_opponent = false;
  bool seen_opponent_head = false;

  /* Check squares neighboring next position.  */
  a_square_index next_pos = state->level->square_move[pp->way][pp->si];
  a_dir i;
  for (i = 0; i < 4; i++) {
    a_square_index idx = state->level->square_move[i][next_pos];
    if (idx == INVALID_INDEX || i == REVERSE_DIR (pp->way)) {
      --free_directions;
    } else {
      a_u8 o = state->square_occupied[idx];
      if (o != 0xff) {
	if ((o & 3) != p) {
	  seen_opponent = true;
	  if (o < 8)
	    seen_opponent_head = true;
	}
	--free_directions;
      } else if (state->square_explo_state[idx] < EXPLOSION_IMMEDIATE + 2
		 && ! pp->invincible) {
	--free_directions;
      }
    }
  }

  (void) callback_data;

  if (! seen_opponent || free_directions > 1) {
    /* If no opponent is wandering around, or there is at least
       two free direction, there is no need to worry.  */
    action->throttle = TH_NORMAL;
    dead_end[p] = false;
    return;
  }

  if (free_directions) {
    /* There are some opponent in the vicinity, but there is at least
       one free direction.

       Speed up if we are near an opponent head (maybe he is trying to
       overtake us).  Don't accelerate otherwise, because we would not
       have the time to stop if he cuts us up.  */
    if (seen_opponent_head)
      action->throttle = TH_SPEEDUP;
    else
      action->throttle = TH_NORMAL;
    dead_end[p] = false;
    return;
  }

  if (! dead_end[p]) {
    /* This is a cul de sac and we haven't yet stopped.
       Better do it now.  */
    int chance = 3;
    assert (seen_opponent && free_directions == 0);
    if (pp->rotozoom)
      --chance;
    if (pp->waves)
      --chance;
    if (rand() % 4 <= chance)
      action->throttle = TH_BRAKE;
    /* Remember we already tried to stop, so we don't give
       another chance to this AI vehicle if it failed to stop.  */
    dead_end[p] = true;
  }
}

static unsigned int
ai_eval_dist (const a_level_state *state, const a_level *lvl, int pos)
{
  a_square_coord curx, cury, distx, disty;
  curx = state->square_coord[pos].x;
  cury = state->square_coord[pos].y;
  if (ai_wrap_left) {
    if (curx <= ai_wrap_x)
      distx = curx + lvl->square_width - ai_target_x;
    else if (curx <= ai_target_x)
      distx = ai_target_x - curx;
    else
      distx = curx - ai_target_x;
  } else {
    if (curx >= ai_wrap_x)
      distx = ai_target_x + lvl->square_width - curx;
    else if (curx <= ai_target_x)
      distx = ai_target_x - curx;
    else
      distx = curx - ai_target_x;
  }
  if (ai_wrap_right) {
    if (cury <= ai_wrap_y)
      disty = cury + lvl->square_height - ai_target_y;
    else if (cury <= ai_target_y)
      disty = ai_target_y - cury;
    else
      disty = cury - ai_target_y;
  } else {
    if (cury >= ai_wrap_y)
      disty = ai_target_y + lvl->square_height - cury;
    else if (cury <= ai_target_y)
      disty = ai_target_y - cury;
    else
      disty = cury - ai_target_y;
  }
  return (distx + disty);
}

/* used by ai_goto_target
 */

#define ai_eval_dir_target_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	    tmp = ai_eval_dir_target (state, lvl, idx);			\
	    if (tmp < mindist) mindist = tmp;				\
    }

#define ai_eval_dir_bonus_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
            tmp = ai_eval_dir_bonus (state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ai_eval_dir_lemming_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	    tmp = ai_eval_dir_lemming (state, lvl, idx);		\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ai_eval_dir_cash_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	    tmp = ai_eval_dir_cash (state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

#define ai_eval_dir_color_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	    tmp = ai_eval_dir_color(state, lvl, idx);			\
	    if (tmp > mindist) mindist = tmp;				\
    }

/*
-> maluses in the position evaluation function
neighb wall ..... : 2pts,
neighb enemy .... : 5pts,
self (old) ...... : 2pts,
self (new = mark) : 1pts,
4pts count as one square in the distance function
*/

static int
ai_eval_neighb_pos (const a_level_state *state, const a_level *lvl,
		    a_dir dir, a_square_index pos)
{
  a_square_index idx;
  unsigned char c;
  idx = lvl->square_move[dir][pos];
  if (idx != INVALID_INDEX) {
    if (square_marks[idx])
      return 1;
    c = state->square_occupied[idx];
    if (c >= 128)
      return 0;
    if ((c & 3) == ai_player)
      return 2;
    else
      return 5;
  }
  return 2;
}

static unsigned int
ai_eval_dir_target (const a_level_state *state, const a_level *lvl,
		    a_square_index pos)
{
  a_u32 mindist;
  a_square_index idx;
  unsigned int tmp;

  ai_cur_depth--;
  if (ai_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = U32_MAX;

    ai_eval_dir_target_inline (D_UP);
    ai_eval_dir_target_inline (D_RIGHT);
    ai_eval_dir_target_inline (D_DOWN);
    ai_eval_dir_target_inline (D_LEFT);

    square_marks[pos] = false;
    ai_cur_depth++;
    return mindist;
  } else {
    ai_cur_depth++;
    tmp = ai_eval_neighb_pos (state, lvl, D_UP, pos)
      + ai_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ai_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ai_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return ((ai_eval_dist (state, lvl, pos) << 16)
	    + (tmp << 14) + ai_max_depth - ai_cur_depth);
  }

}

static int
ai_eval_dir_lemming (const a_level_state *state, const a_level *lvl,
		     a_square_index pos)
{
  int mindist;
  a_square_index idx;
  int tmp, tmp2;
  a_lemming *tmppti;

  ai_cur_depth--;
  if (ai_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = 0;
    tmppti = state->square_lemmings_list[pos];
    if (tmppti) {
      if (tmppti->color == ai_player)
	tmp2 = -100;
      else
	tmp2 = 20;
    } else
      tmp2 = 0;

    ai_eval_dir_lemming_inline (D_UP);
    ai_eval_dir_lemming_inline (D_RIGHT);
    ai_eval_dir_lemming_inline (D_DOWN);
    ai_eval_dir_lemming_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ai_cur_depth); */

    square_marks[pos] = false;
    ai_cur_depth++;
    return mindist;
  } else {
    ai_cur_depth++;
    tmp = ai_eval_neighb_pos (state, lvl, D_UP, pos)
      + ai_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ai_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ai_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }

}

static int
ai_eval_dir_color (const a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ai_cur_depth--;
  if (ai_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = 0;
    d = state->square_object[pos];
    tmp2 = 0;
    if (d >= 0) {
      if (d <= 4) {
	if (d == ai_player)
	  tmp2 = 100;
	else
	  tmp2 = -200;
      } else if (d <= 12) {
	if (d == ai_player + 8)
	  tmp2 = -200;
	else
	  tmp2 = 100;
      } else if (d == 16)
	tmp2 = 100;
      else if (d == 24)
	tmp2 = -40;
    }

    ai_eval_dir_color_inline (D_UP);
    ai_eval_dir_color_inline (D_RIGHT);
    ai_eval_dir_color_inline (D_DOWN);
    ai_eval_dir_color_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ai_cur_depth); */

    square_marks[pos] = false;
    ai_cur_depth++;
    return mindist;
  } else {
    ai_cur_depth++;
    tmp = ai_eval_neighb_pos (state, lvl, D_UP, pos)
      + ai_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ai_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ai_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }
}

static int
ai_eval_dir_cash (const a_level_state *state, const a_level *lvl,
		  a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ai_cur_depth--;
  if (ai_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = 0;
    d = state->square_object[pos];

    if (d >= 0)
      tmp2 = 500;
    else
      tmp2 = -20;

    ai_eval_dir_cash_inline (D_UP);
    ai_eval_dir_cash_inline (D_RIGHT);
    ai_eval_dir_cash_inline (D_DOWN);
    ai_eval_dir_cash_inline (D_LEFT);

    mindist += tmp2;		/* *(5+ai_cur_depth); */

    square_marks[pos] = false;
    ai_cur_depth++;
    return mindist;
  } else {
    ai_cur_depth++;
    tmp = ai_eval_neighb_pos (state, lvl, D_UP, pos)
      + ai_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ai_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ai_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 2);
  }

}

static int
ai_eval_dir_bonus (const a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  ai_cur_depth--;
  if (ai_cur_depth != 0) {
    int tmp2 = 0;
    int mindist = 0;
    a_tile_index d = state->square_tile[pos];
    a_square_index idx;
    int tmp;
    square_marks[pos] = true;
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
    ai_eval_dir_bonus_inline (D_UP);
    ai_eval_dir_bonus_inline (D_RIGHT);
    ai_eval_dir_bonus_inline (D_DOWN);
    ai_eval_dir_bonus_inline (D_LEFT);

    mindist += tmp2 * (5 + ai_cur_depth) /* /ai_max_depth */ ;

    if (tmp2)
      tile_bonus_cpu[state->square_tile[pos]] = 0;
    square_marks[pos] = false;
    ai_cur_depth++;
    return mindist;
  } else {
    int tmp;
    ai_cur_depth++;
    tmp = ai_eval_neighb_pos (state, lvl, D_UP, pos)
      + ai_eval_neighb_pos (state, lvl, D_RIGHT, pos)
      + ai_eval_neighb_pos (state, lvl, D_DOWN, pos)
      + ai_eval_neighb_pos (state, lvl, D_LEFT, pos);
    return -(tmp << 3);
  }

}

/* give the *way* to follow to get a given position */

#define ai_goto_target_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	  ai_cur_depth=ai_max_depth;					\
	  tmp[dir] = ai_eval_dir_target (state, lvl, idx);		\
	  if (tmp[dir] < mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ai_goto_bonus_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	  ai_cur_depth = ai_max_depth;					\
	  tmp[dir] = ai_eval_dir_bonus(state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ai_goto_lemming_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	  ai_cur_depth = ai_max_depth;					\
	  tmp[dir] = ai_eval_dir_lemming(state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ai_goto_color_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	  ai_cur_depth = ai_max_depth;					\
	  tmp[dir] = ai_eval_dir_color (state, lvl, idx);		\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

#define ai_goto_cash_inline(dir)					\
    idx = lvl->square_move[dir][pos];					\
    if (idx != INVALID_INDEX)						\
    if ((state->square_occupied[idx] == 0xff) &&			\
       ((state->square_explo_state[idx] >= EXPLOSION_IMMEDIATE + 2)	\
        || ai_is_invincible)) {						\
	  ai_cur_depth = ai_max_depth;					\
	  tmp[dir] = ai_eval_dir_cash(state, lvl, idx);			\
	  if (tmp[dir] > mindist) {					\
		mindist = tmp[dir];					\
		mindir = dir;						\
	  }								\
    }

static void
ai_goto_target (const a_level_state *state, int c,
		an_opponent_action *action,
		void *callback_data)
{
  a_square_index idx, pos;
  a_u32 tmp[4] = { U32_MAX, U32_MAX, U32_MAX, U32_MAX };
  a_u32 mindist = U32_MAX;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  const a_player *const pp = state->player[c];
  (void) callback_data;

  ai_player = c;
  ai_max_depth = pp->ai_max_depth;
  ai_target_x = state->player[pp->target]->sx;
  ai_target_y = state->player[pp->target]->sy;
  ai_wrap_x = ai_target_x + lvl->tile_width;
  if (ai_wrap_x >= lvl->square_width) {
    ai_wrap_x -= lvl->square_width;
    ai_wrap_left = 1;
  } else
    ai_wrap_left = 0;
  ai_wrap_y = ai_target_y + lvl->tile_height;
  if (ai_wrap_y >= lvl->square_height) {
    ai_wrap_y -= lvl->square_height;
    ai_wrap_right = 1;
  } else
    ai_wrap_right = 0;

  ai_is_invincible = (pp->invincible != 0);
  pos = pp->si;
  ai_goto_target_inline (D_UP);
  ai_goto_target_inline (D_RIGHT);
  ai_goto_target_inline (D_DOWN);
  ai_goto_target_inline (D_LEFT);
  if (tmp[pp->way] == tmp[mindir])
    action->dir = pp->way;
  else
    action->dir = mindir;
}

static void
ai_goto_nearest_bonus (const a_level_state *state, int c,
		       an_opponent_action *action,
		       void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  const a_player *const pp = state->player[c];
  (void) callback_data;

  ai_player = c;
  ai_max_depth = pp->ai_max_depth;
  ai_is_invincible = (pp->invincible != 0);
  pos = pp->si;
  ai_goto_bonus_inline (D_UP);
  ai_goto_bonus_inline (D_RIGHT);
  ai_goto_bonus_inline (D_DOWN);
  ai_goto_bonus_inline (D_LEFT);
  if (tmp[pp->way] == tmp[mindir])
    action->dir = pp->way;
  else
    action->dir = mindir;
}

static void
ai_goto_nearest_lemming (const a_level_state *state, int c,
			 an_opponent_action *action,
			 void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  const a_player *const pp = state->player[c];
  (void) callback_data;

  ai_player = c;
  ai_max_depth = pp->ai_max_depth;
  ai_is_invincible = (pp->invincible != 0);
  pos = pp->si;
  ai_goto_lemming_inline (D_UP);
  ai_goto_lemming_inline (D_RIGHT);
  ai_goto_lemming_inline (D_DOWN);
  ai_goto_lemming_inline (D_LEFT);
  if (tmp[pp->way] == tmp[mindir])
    action->dir = pp->way;
  else
    action->dir = mindir;
}

static void
ai_goto_nearest_color (const a_level_state *state, int c,
		       an_opponent_action *action,
		       void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  const a_player *const pp = state->player[c];
  (void) callback_data;

  ai_player = c;
  ai_max_depth = pp->ai_max_depth;
  ai_is_invincible = (pp->invincible != 0);
  pos = pp->si;
  ai_goto_color_inline (D_UP);
  ai_goto_color_inline (D_RIGHT);
  ai_goto_color_inline (D_DOWN);
  ai_goto_color_inline (D_LEFT);
  if (tmp[pp->way] == tmp[mindir])
    action->dir = pp->way;
  else
    action->dir = mindir;
}

static void
ai_goto_nearest_cash (const a_level_state *state, int c,
		      an_opponent_action *action,
		      void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  const a_player *const pp = state->player[c];
  (void) callback_data;

  ai_player = c;
  ai_max_depth = pp->ai_max_depth;
  ai_is_invincible = (pp->invincible != 0);
  pos = pp->si;
  ai_goto_cash_inline (D_UP);
  ai_goto_cash_inline (D_RIGHT);
  ai_goto_cash_inline (D_DOWN);
  ai_goto_cash_inline (D_LEFT);
  if (tmp[pp->way] == tmp[mindir])
    action->dir = pp->way;
  else
    action->dir = mindir;
}

static an_opponent_sig ai_standard_quest = {
  "standard AI for Quest mode",
  OT_QUEST,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ai_goto_nearest_bonus,
  &ai_throttle
};

static an_opponent_sig ai_standard_deathm = {
  "standard AI for Death Match",
  OT_QUEST | OT_DEATHM,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ai_goto_target,
  &ai_throttle
};

static an_opponent_sig ai_standard_killem = {
  "standard AI for Kill'em all",
  OT_KILLEM,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ai_goto_nearest_lemming,
  &ai_throttle
};

static an_opponent_sig ai_standard_color = {
  "standard AI for Color",
  OT_COLOR,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ai_goto_nearest_color,
  &ai_throttle
};

static an_opponent_sig ai_standard_tcash = {
  "standard AI for Time Ca$h",
  OT_TCASH,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ai_goto_nearest_cash,
  &ai_throttle
};


void stdai_LTX_initialize (void);
void
stdai_LTX_initialize (void)
{
  opponent_register (&ai_standard_quest);
  opponent_register (&ai_standard_deathm);
  opponent_register (&ai_standard_killem);
  opponent_register (&ai_standard_color);
  opponent_register (&ai_standard_tcash);
}

void stdai_LTX_finalize (void);
void
stdai_LTX_finalize (void)
{
  opponent_unregister (&ai_standard_quest);
  opponent_unregister (&ai_standard_deathm);
  opponent_unregister (&ai_standard_killem);
  opponent_unregister (&ai_standard_color);
  opponent_unregister (&ai_standard_tcash);
}
