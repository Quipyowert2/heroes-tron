/*------------------------------------------------------------------------.
| Copyright 2000, 2002  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                         |
| This file is part of Heroes.                                            |
|                                                                         |
| Heroes is free software; you can redistribute it and/or modify it under |
| the terms of the GNU General Public License version 2 as published by   |
| the Free Software Foundation.                                           |
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
#include "statepriv.h"
#include "sfx.h"
#include "argv.h"
#include "debugmsg.h"

#include "bonus.h"		/* FIXME: Get rid of this.  */
#include "heroes.h"		/* FIXME: Get rid of this.  */
#include "renderdata.h"		/* FIXME: Get rid of this.  */

/* density of bonuses in different modes */

/* L+  L-  S+ S- R#   C  ZZ !!  -1  T+  T- EL []  X XL ~~  $$ */
const int bonus_density[5][17] =
{ {40, 10, 12, 8, 8, 40,  6, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0},	/* quest */
  { 0,  0, 10, 7, 7, 20, 12, 8, 10, 10, 11, 0, 6, 7, 2, 7, 0},	/* deathm */
  {25, 10, 12, 8, 8, 40,  6, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0},	/* killem */
  {25, 10, 12, 8, 8, 20,  6, 8, 10, 16, 16, 0, 8, 7, 4, 7, 25},	/* tcash */
  {25, 10, 12, 8, 8, 30,  6, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0}    /* color */
};

/* interest of each bonus, for the CPU controled vehicles */
int bonus_points[2][17] =
{ {20, -15, 15, -10, 5, 18, 19, -10, 0, 0, -5, 50, 5, 0, 8, 0, 25}, /* orchid */
  {-15, 10, 0, 10, 5, -5, 0, 8, 8, -5, 5, -20, -5, 9, -10, 9, -25} /* peach */
};

/* FIXME: Move most of these to state->private.  */

static int bonus_proba[17];	/* FIXME: What's the english for
				   "fonction de r\'epartition"? */
static int bonus_proba_sum = 0;

int *bonus_time;
int *bonus_list;

static int bonus_total_nbr, bonus_real_nbr;
static int next_bonus_to_update;

static unsigned char
random_bonus (void)
{
  int t;
  unsigned char b;
  b = 0;
  t = (rand () % bonus_proba_sum) + 1;
  while (t > bonus_proba[b])
    b++;
  return (b);
}

static void
add_bonus (a_level_state *state, const a_level *lvl,
	   int pos_in_list, unsigned char what)
{
  int pos;

  do
    pos = rand () % lvl->tile_count;
  while (state->tile_bonus[pos] != 0);

  state->tile_bonus[pos] = what;
  bonus_list[pos_in_list] = pos;
  bonus_time[pos_in_list] = event_time + (rand () % 511) - 256;

  dmsg (D_BONUS, "Add bonus: type=%u, pos=%u, pos_in_list=%u",
	what, pos, pos_in_list);

  --what;
  /* update foreground data for rendering */
  if (what == 16)
    fg_data[pos].big_dollar = 1;
  else {
    if (what & 128)
      fg_data[pos].bonus = bonus_rle[1][what & 127];
    else
      fg_data[pos].bonus = bonus_rle[0][what];
  }
}

void
add_random_bonus (a_level_state *state, const a_level *lvl, int pos_in_list)
{
  unsigned char what;

  what = random_bonus ();
  if (what != 16)		/* yellow `$$' bonuses do not exist */
    if (!(rand () & 3))
      what |= 128;

  add_bonus (state, lvl, pos_in_list, what + 1);
}

void
rem_bonus (a_level_state *state, const a_level *lvl, int pos)
{
  int i = bonus_real_nbr;

  dmsg (D_BONUS, "Remove bonus: pos = %d", pos);

  /* find the bonus position in the list of bonuses */
  do
    i--;
  while (bonus_list[i] != pos);

  /* remove the bonus */
  state->tile_bonus[pos] = 0;

  /* don't draw it anymore */
  fg_data[pos].bonus = 0;
  fg_data[pos].big_dollar = 0;

  /* add a new bonus, at the same position in the list */
  add_random_bonus (state, lvl, i);
}

static void
reset_bonus_mode (int mode)
{
  int i;

  bonus_proba_sum = 0;
  for (i = 0; i < 17; i++)
    bonus_proba_sum =
      bonus_proba[i] = bonus_density[mode][i] + bonus_proba_sum;
}

static void
mark_unreachable_places (a_level_state *state, const a_level *lvl)
{
  a_tile_index i;

  for (i = 0; i < lvl->tile_count; ++i) {
    a_square_index s = TILE_INDEX_TO_SQR_INDEX (lvl, i);
    /* if the place can't be reached, or is a corridor, don't
       put a bonus */
    if ((lvl_tile_type (lvl, i) == T_OUTWAY)
	|| (lvl->square_walls_out[SQR0 (lvl, s)] & (DM_DOWN | DM_RIGHT))
	|| (lvl->square_walls_out[SQR1 (lvl, s)] & (DM_DOWN | DM_LEFT))
	|| (lvl->square_walls_out[SQR2 (lvl, s)] & (DM_UP | DM_RIGHT))
	|| (lvl->square_walls_out[SQR3 (lvl, s)] & (DM_UP | DM_LEFT)))
      state->tile_bonus[i] = 0xff;
  }
}

int
init_bonuses_level (a_level_state *state, const a_level *lvl)
{
  uninit_bonuses_level (state);	/* just in case */

  dmsg (D_BONUS, "Initialize bonuses for level.");

  reset_bonus_mode (state->game_mode);

  XCALLOC_ARRAY (state->tile_bonus, lvl->tile_count);
  XCALLOC_ARRAY (state->private->tile_bonus_cpu, lvl->tile_count);

  bonus_total_nbr = (lvl->tile_count / 90) + 3;
  bonus_real_nbr = bonus_total_nbr - 2;
  next_bonus_to_update = 0;

  dmsg (D_BONUS, "bonus_total_nbr=%d, bonus_real_nbr=%d",
	bonus_total_nbr, bonus_real_nbr);

  XMALLOC_ARRAY (bonus_time, bonus_total_nbr);
  XMALLOC_ARRAY (bonus_list, bonus_total_nbr);

  mark_unreachable_places (state, lvl);

  render_init_bonus_level ();
  return 0;
}

void
uninit_bonuses_level (a_level_state *state)
{
  dmsg (D_BONUS, "Uninitialize bonuses for level.");

  XFREE0 (state->tile_bonus);
  XFREE0 (state->private->tile_bonus_cpu);
  XFREE0 (bonus_time);
  XFREE0 (bonus_list);
}

void
spread_bonuses (a_level_state *state, const a_level *lvl)
{
  int i;

  dmsg (D_BONUS, "Spread bonuses over level.");

  for (i = bonus_real_nbr - 1; i >= 0; i--)
    add_random_bonus (state, lvl, i);
}

void
add_end_level_bonuses (a_level_state *state, const a_level *lvl)
{
  if (bonus_real_nbr != bonus_total_nbr) {
    int i;

    dmsg (D_BONUS, "Add end-level bonuses.");

    add_bonus (state, lvl, bonus_real_nbr++, 12);
    add_bonus (state, lvl, bonus_real_nbr++, 12 + 128);
    for (i = 11; i < 17; i++)
      bonus_proba[i] += 16;
    bonus_proba_sum += 16;
  }
}


void
apply_bonus (a_level_state *state, const a_level *lvl, int pl, char bonus)
{
  static char txt_tmp[20];

  dmsg (D_BONUS, "Player %u got bonus %u.", pl, bonus);

  if (bonus == 5) {
    bonus = random_bonus () + 1;
    if (bonus == 5)
      bonus++;
  }

  if (state->player[pl].cpu == 2)
    event_sfx (19 + bonus);
  switch (bonus) {
  case 1:
    grow_trail (state, lvl, pl, 5);
    sprintf (txt_tmp, _("SIZE IS %d"), state_trail_size (state, pl));
    set_txt_bonus (pl, txt_tmp, 150);
    break;
  case 2:
    shrink_trail (state, pl, 5);
    sprintf (txt_tmp, _("SIZE IS %d"), state_trail_size (state, pl));
    set_txt_bonus (pl, txt_tmp, 150);
    break;
  case 3:
    state->player[pl].speedup = 500;
    set_txt_bonus (pl, _("SPEEDED UP"), 150);
    break;
  case 4:
    state->player[pl].speedup = -500;
    set_txt_bonus (pl, _("SPEEDED DOWN"), 150);
    break;
  case 6:
    {
      int i;
      i = rand () & 255;
      state->player[pl].score += i;
      sprintf (txt_tmp, _("GET %dPTS"), i);
      set_txt_bonus (pl, txt_tmp, 150);
    }
    break;
  case 7:
    set_txt_bonus (pl, _("FIRE TRAIL!"), 150);
    state->player[pl].fire_trail += 2000;
    break;
  case 8:
    state->player[pl].notify_delay = 1;
    break;
  case 9:
    state->player[pl].inversed_controls = 500;
    break;
  case 10:
    if (state->player[pl].turbo_level > 1024 - 512)
      state->player[pl].turbo_level = 1024;
    else
      state->player[pl].turbo_level += 512;
    set_txt_bonus (pl, _("GET TURBO+"), 150);
    break;
  case 11:
    if (state->player[pl].turbo_level > 256)
      state->player[pl].turbo_level -= 256;
    else
      state->player[pl].turbo_level = 0;
    set_txt_bonus (pl, _("GET TURBO-"), 150);
    break;
  case 12:
    if (state_trail_size (state, pl) >= 10)
      state_level_set_exit_code (state, pl + 1);
    break;
  case 13:
    state->player[pl].invincible = 350;
    set_txt_bonus (pl, _("INVINCIBLE!"), 150);
    break;
  case 14:
    if (state->player[pl].waves == 0 || doublefx != 0) {
      state->player[pl].rotozoom += 1024;
      if (state->player[pl].rotozoom == 0)
	state->player[pl].rotozoom_direction = rand () & 1;
    }
    break;
  case 15:
    if (state->player[pl].lifes < 100) {
      state->player[pl].lifes++;
      set_txt_bonus (pl, _("EXTRA-LIFE!"), 150);
    }
    break;
  case 16:
    if (state->player[pl].rotozoom == 0 || doublefx != 0)
      state->player[pl].waves += 1024;
    break;
  case 17:
    state->player[pl].cash += 10;
    state->player[pl].score += 50;
    break;
  default:
    assert (0 /* unknown bonus! */ );
    break;
  }
}

/* only one bonus is updated at each frame (there is no hurry) */
void
update_bonuses (a_level_state *state, const a_level *lvl)
{
  if (bonus_time[next_bonus_to_update] + 25 * 70 <= event_time) {
    int bonus_pos = bonus_list[next_bonus_to_update];

    dmsg (D_BONUS, "Expired bonus: pos=%u, pos_in_list=%u",
	  bonus_pos, next_bonus_to_update);

    /* erase the bonus */
    state->tile_bonus[bonus_pos] = 0;
    /* don't draw it anymore */
    fg_data[bonus_pos].bonus = 0;
    fg_data[bonus_pos].big_dollar = 0;
    /* add a new bonus, at the same position in the list */
    add_random_bonus (state, lvl, next_bonus_to_update);
  }
  next_bonus_to_update++;
  if (next_bonus_to_update >= bonus_real_nbr)
    next_bonus_to_update = 0;
}
