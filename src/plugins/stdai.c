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
#include "state.h"
#include "hooks.h"
#include "opponents.h"

char ia_max_depth;
char ia_cur_depth;
char ia_is_invincible;
char ia_player;
a_square_coord ia_target_x, ia_target_y;
a_square_coord ia_wrap_x, ia_wrap_y;
char ia_wrap_left, ia_wrap_right;

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

  unsigned free_directions = 4;
  bool seen_opponent = false;
  bool seen_opponent_head = false;

  /* Check squares neighboring next position.  */
  a_square_index next_pos =
    state->level->square_move[state->player[p].way][state->player[p].pos];
  a_dir i;
  for (i = 0; i < 4; i++) {
    a_square_index idx = state->level->square_move[i][next_pos];
    if (idx == INVALID_INDEX || i == REVERSE_DIR (state->player[p].way)) {
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
		 && ! state->player[p].invincible) {
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
    if (state->player[p].rotozoom)
      --chance;
    if (state->player[p].waves)
      --chance;
    if (rand() % 4 <= chance)
      action->throttle = TH_BRAKE;
    /* Remember we already tried to stop, so we don't give
       another chance to this AI vehicle if it failed to stop.  */
    dead_end[p] = true;
  }
}

static unsigned int
ia_eval_dist (const a_level_state *state, const a_level *lvl, int pos)
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
self (new = mark) : 1pts,
4pts count as one square in the distance function
*/

static int
ia_eval_neighb_pos (const a_level_state *state, const a_level *lvl,
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
    if ((c & 3) == ia_player)
      return 2;
    else
      return 5;
  }
  return 2;
}

static unsigned int
ia_eval_dir_target (const a_level_state *state, const a_level *lvl,
		    a_square_index pos)
{
  a_u32 mindist;
  a_square_index idx;
  unsigned int tmp;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = U32_MAX;

    ia_eval_dir_target_inline (D_UP);
    ia_eval_dir_target_inline (D_RIGHT);
    ia_eval_dir_target_inline (D_DOWN);
    ia_eval_dir_target_inline (D_LEFT);

    square_marks[pos] = false;
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
ia_eval_dir_lemming (const a_level_state *state, const a_level *lvl,
		     a_square_index pos)
{
  int mindist;
  a_square_index idx;
  int tmp, tmp2;
  a_lemming *tmppti;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_marks[pos] = true;
    mindist = 0;
    tmppti = state->square_lemmings_list[pos];
    if (tmppti) {
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

    square_marks[pos] = false;
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
ia_eval_dir_color (const a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_marks[pos] = true;
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

    square_marks[pos] = false;
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
ia_eval_dir_cash (const a_level_state *state, const a_level *lvl,
		  a_square_index pos)
{
  signed int mindist;
  a_square_index idx;
  int d, tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_marks[pos] = true;
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

    square_marks[pos] = true;
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
ia_eval_dir_bonus (const a_level_state *state, const a_level *lvl,
		   a_square_index pos)
{
  ia_cur_depth--;
  if (ia_cur_depth != 0) {
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
    ia_eval_dir_bonus_inline (D_UP);
    ia_eval_dir_bonus_inline (D_RIGHT);
    ia_eval_dir_bonus_inline (D_DOWN);
    ia_eval_dir_bonus_inline (D_LEFT);

    mindist += tmp2 * (5 + ia_cur_depth) /* /ia_max_depth */ ;

    if (tmp2)
      tile_bonus_cpu[state->square_tile[pos]] = 0;
    square_marks[pos] = false;
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

static void
ia_goto_target (const a_level_state *state, int c,
		an_opponent_action *action,
		void *callback_data)
{
  a_square_index idx, pos;
  a_u32 tmp[4] = { U32_MAX, U32_MAX, U32_MAX, U32_MAX };
  a_u32 mindist = U32_MAX;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  (void) callback_data;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_target_x = state->player[state->player[c].target].x2;
  ia_target_y = state->player[state->player[c].target].y2;
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
    action->dir = state->player[c].way;
  else
    action->dir = mindir;
}

static void
ia_goto_nearest_bonus (const a_level_state *state, int c,
		       an_opponent_action *action,
		       void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  (void) callback_data;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_bonus_inline (D_UP);
  ia_goto_bonus_inline (D_RIGHT);
  ia_goto_bonus_inline (D_DOWN);
  ia_goto_bonus_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    action->dir = state->player[c].way;
  else
    action->dir = mindir;
}

static void
ia_goto_nearest_lemming (const a_level_state *state, int c,
			 an_opponent_action *action,
			 void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  (void) callback_data;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_lemming_inline (D_UP);
  ia_goto_lemming_inline (D_RIGHT);
  ia_goto_lemming_inline (D_DOWN);
  ia_goto_lemming_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    action->dir = state->player[c].way;
  else
    action->dir = mindir;
}

static void
ia_goto_nearest_color (const a_level_state *state, int c,
		       an_opponent_action *action,
		       void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  (void) callback_data;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_color_inline (D_UP);
  ia_goto_color_inline (D_RIGHT);
  ia_goto_color_inline (D_DOWN);
  ia_goto_color_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    action->dir = state->player[c].way;
  else
    action->dir = mindir;
}

static void
ia_goto_nearest_cash (const a_level_state *state, int c,
		      an_opponent_action *action,
		      void *callback_data)
{
  a_square_index idx, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  a_dir mindir = 0;
  const a_level *lvl = state->level;
  (void) callback_data;

  ia_player = c;
  ia_max_depth = state->player[c].ia_max_depth;
  ia_is_invincible = (state->player[c].invincible != 0);
  pos = state->player[c].pos;
  ia_goto_cash_inline (D_UP);
  ia_goto_cash_inline (D_RIGHT);
  ia_goto_cash_inline (D_DOWN);
  ia_goto_cash_inline (D_LEFT);
  if (tmp[state->player[c].way] == tmp[mindir])
    action->dir = state->player[c].way;
  else
    action->dir = mindir;
}

an_opponent_sig ai_standard_quest = {
  "standard AI for Quest mode",
  OT_QUEST,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ia_goto_nearest_bonus,
  &ai_throttle
};

an_opponent_sig ai_standard_deathm = {
  "standard AI for Death Match",
  OT_QUEST | OT_DEATHM,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ia_goto_target,
  &ai_throttle
};

an_opponent_sig ai_standard_killem = {
  "standard AI for Kill'em all",
  OT_KILLEM,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ia_goto_nearest_lemming,
  &ai_throttle
};

an_opponent_sig ai_standard_color = {
  "standard AI for Color",
  OT_COLOR,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ia_goto_nearest_color,
  &ai_throttle
};

an_opponent_sig ai_standard_tcash = {
  "standard AI for Time Ca$h",
  OT_COLOR,
  &ai_level_initialize,
  &ai_level_finalize,
  0,
  0,
  &ia_goto_nearest_cash,
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
