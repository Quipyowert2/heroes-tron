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
#include "ai.h"

#include "bonus.h"		/* FIXME: Get rid of this.  */
#include "explosions.h"		/* FIXME: Get rif of this.  */
#include "sfx.h"		/* FIXME: Get rif of this.  */
#include "argv.h"		/* FIXME: Get rif of this.  */


/* search for a free direction */
static void
find_free_way (a_level_state *state, const a_level *lvl, int c)
{
  int d = 0, n = 0, o[4] = { 0xff, 0xff, 0xff, 0xff }, e;
  int i, m;
  a_dir f;

  m = state->player[c].x2 + state->player[c].y2 * lvl->square_width;
  e = 1;
  for (i = 0; i < 4; i++) {
    a_square_index idx = lvl->square_move[i][m];
    if (idx != INVALID_INDEX)
      o[i] = state->square_occupied[idx];

    /* Forbid turn back.  This is usually not needed because the
       square behind the vehicle is already occupied, but in some
       tunnel configurations this may not be the case. */
    o[REVERSE_DIR (state->player[c].way)] = c;

    if (o[i] != 0xff || idx == INVALID_INDEX)
      d |= e;
    e += e;
  }

  f = state->player[c].next_way;

  /* Since the auto pilot is deactivated in case a player runs into a
     fire trail, it's possible for a human player to do a one-eighty
     turn and crash into his own tail. That can be quite annoying in
     case you want to go back one square left or right of your current
     lane and you're too fast pressing the buttons.

     Explicitly ignore the new direction in this case.  */
  if (f == REVERSE_DIR (state->player[c].way))
    state->player[c].next_way = state->player[c].way;

  /* If the way is free the autopilot has nothing to do.  */
  if (!(d & (1 << f)))
    return;

  /* If we reach this place, NEXT_WAY cannot be taken because there is
     a wall or someone else.  Therefore we will want to find some
     other direction automatically.  */

  if (state->player[c].cpu & 2) {
    /* The autopilot for human players does not work against fire trails,
       that would be too easy :).
       If NEXT_WAY would lead to a fired square, return immediately,
       unless the player is invincible, in which case the autopilot still
       apply.  */
    a_square_index idx = lvl->square_move[f][m];
    if (idx != INVALID_INDEX
	&& state->square_explo_state[idx] <= EXPLOSION_IMMEDIATE
	&& !state->player[c].invincible)
      return;
  }

  e = o[state->player[c].next_way];

  /* when a trail force someone to turn, the owner of this trail is credited */
  if ((e & 3) != c && (!state->private->level_is_finished) && e != 0xff
      && (state->player[e & 3].spec != 0xde))
    state->player[e & 3].score += 5;

  if (!(d & (1 << state->player[c].old_old_way))) {
    state->player[c].next_way = state->player[c].old_old_way;
    return;
  }

  if (!(d & (1 << state->player[c].old_way))) {
    state->player[c].next_way = state->player[c].old_way;
    return;
  }

  if (state->player[c].spec != T_ICE)
    for (i = 1; i != 16; i += i)
      if (!(d & i))
	n++;

  e = d;
  if (n != 0) {
    n = (char) (1 + rand () % n);
    for (i = 0; n != 0; i++, d >>= 1)
      if (!(d & 1))
	n--;
    state->player[c].next_way = (char) (i - 1);
/*  if (w2d[i-1]&e) fatal_error("find_free_way() return nonsense !"); */
    assert (((1 << (i - 1)) & e) == 0);
  } else
    state->player[c].spec = 0xff;
}


/* * * * * * * * * * * * * * * * * * * * *\
 * Handling of players moves, collisions, *
 * bonus effects, lemmings dies, etc.     *
 \* * * * * * * * * * * * * * * * * * * **/

/* FIXME: there are many things here that should be moved in a
   separate function (e.g. `update_game') in order to be run only once
   per update, and not four times.  FIXME: Split this in smaller
   functions.  E.g. one function per mode.  */
void
update_player (a_level_state *state, const a_level *lvl, unsigned c)
{
  a_square_index idx;
  a_tile_index d;
  int l;
  unsigned i;
  a_square_index d2;
  int t;
  a_lemming *tmppti;

  if ((state->player[c].score_delta >> 2) < state->player[c].score) {
    state->player[c].score_delta++;
    /* 1 life every 10.000 points */
    if (state->player[c].score_delta % (10000 << 2) == 0)
      apply_bonus (state, lvl, c, 15);
  }
/* if ((state->player[c].score_delta>>2)>state->player[c].score) state->player[c].score_delta--; */
  if (state->player[c].turbo_level_delta < state->player[c].turbo_level) {
    state->player[c].turbo_level_delta += 8;
    if (state->player[c].turbo_level_delta > state->player[c].turbo_level)
      state->player[c].turbo_level_delta = state->player[c].turbo_level;
  } else if (state->player[c].turbo_level_delta > state->player[c].turbo_level) {
    state->player[c].turbo_level_delta -= 8;
    if (state->player[c].turbo_level_delta < state->player[c].turbo_level)
      state->player[c].turbo_level_delta = state->player[c].turbo_level;
  }

  if (state->player[c].invincible > 0)
    state->player[c].invincible--;
  if (state->game_mode >= M_TCASH) {
    if (state->player[c].time > 0) {
      state->player[c].time--;
/* stop the game if the player is alone */
/*       if ((!level_is_finished) && */
/*           (state->player[(c+1)&3].spec==0xde) && */
/*           (state->player[(c+2)&3].spec==0xde) && */
/*           (state->player[(c+3)&3].spec==0xde)) { level_is_finished=c+1; return; } */
    } else if (!state->private->level_is_finished) {
      state->player[c].spec = 0xde;
      erase_trail (state, lvl, c);
      /* stop the game if all human players are dead or
	 if there is no more colors or dollars */
      if ((!(((state->player[0].cpu & 2) && (state->player[0].time))
	     || ((state->player[1].cpu & 2) && (state->player[1].time))
	     || ((state->player[2].cpu & 2) && (state->player[2].time))
	     || ((state->player[3].cpu & 2) && (state->player[3].time))))
	  || (state->private->objects_nbr == 0)) {
	/* KLUGE: mark all players whose time is 0 as dead.  This is
	   needed because many players can reach 0 simultaneously but
	   this block is only run for the first player when this is
	   discovered.  */
	for (i = 0; i < 4; ++i)
	  if (state->player[i].time == 0) {
	    state->player[i].spec = 0xde;
	    erase_trail (state, lvl, i);
	  }
	/* find out the richest player and set level_is_finished accordingly */
	state->private->level_is_finished = 0;
	for (i = 1; i < 4; i++)
	  if (state->player[i].cash > state->player[state->private->level_is_finished].cash)
	    state->private->level_is_finished = i;
	++state->private->level_is_finished;
      }
    }
  }
  update_player_bonus_vars (c);
  d = (state->player[c].x2 >> 1) + (state->player[c].y2 >> 1) * lvl->tile_width;
  if (state->player[c].rotozoom != 0)
    state->player[c].rotozoom--;
  if (state->player[c].waves != 0) {
    state->player[c].waves--;
    if (state->player[c].waves > 128 && state->player[c].waves_begin < 128)
      state->player[c].waves_begin++;
    if (state->player[c].waves < 128 && state->player[c].waves_begin > 0)
      state->player[c].waves_begin--;
  }
  if (state->player[c].fire_trail)
    --state->player[c].fire_trail;

  if (state->player[c].spec == 0xde)
    return;

  if (state->player[c].inversed_controls > 0) {
    char txt_tmp[128];
    state->player[c].inversed_controls--;
    sprintf (txt_tmp, _("INVERTED %d"), state->player[c].inversed_controls / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }

  if (state->player[c].delay == 0) {
    /* Adjust speed of AI-controled vehicles.  */
    if (cpuon && ((state->player[c].cpu & 2) == 0))
      ai_throttle (state, lvl, c);
    if (state->player[c].turbo != 1 && state->player[c].turbo_level > 0
	&& state->player[c].speedup == 0) {
      state->player[c].vitt = (state->player[c].v + state->player[c].vi) * state->player[c].turbo;
      state->player[c].turbo_level -= 2;
    } else if (state->player[c].speedup > 0) {
      state->player[c].vitt = (state->player[c].v + state->player[c].vi) << 1;
      state->player[c].speedup--;
    } else if (state->player[c].speedup < 0) {
      state->player[c].vitt = (state->player[c].v + state->player[c].vi) >> 1;
      state->player[c].speedup++;
    } else
      state->player[c].vitt = (state->player[c].v + state->player[c].vi);
    if (state->player[c].vitp < state->player[c].vitt)
      if (state->player[c].vitp + 512 < state->player[c].vitt)
	state->player[c].vitp += 512;
      else
	state->player[c].vitp = state->player[c].vitt;
    else if (state->player[c].vitp > state->player[c].vitt) {
      if (state->player[c].vitp + 512 > state->player[c].vitt)
	state->player[c].vitp -= 512;
      else
	state->player[c].vitp = state->player[c].vitt;
    }
    state->player[c].d.e += state->player[c].vitp;
  } else {
    char txt_tmp[128];
    state->player[c].delay--;
    sprintf (txt_tmp, _("STOPPED %d"), state->player[c].delay / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }


  if (state->player[c].d.h.h != 0 || state->player[c].delay == 1) {

/**** handling of trails ****/
    if (state->player[c].delay == 0) {
      int a;
      l = state->player[c].x2 + state->player[c].y2 * lvl->square_width;
      state->square_occupied[l] = SQOC_TRAIL(c);
      state->private->trail_offset[c] =
	(state->private->trail_offset[c] - 1) & (maxq - 1);
      state->private->trail_pos[c][state->private->trail_offset[c]] = l;
      state->private->trail_way[c][state->private->trail_offset[c]] =
	state->square_way[l] =
	DIR8_PAIR (state->player[c].way, state->player[c].old_way);
      a = (state->private->trail_offset[c] + state->private->trail_size[c]) & (maxq - 1);
      if (state->private->trail_pos[c][a]
	  != state->private->trail_pos[c][(state->private->trail_offset[c]
					  + state->private->trail_size[c] - 1)
					& (maxq - 1)]) {
	if (state->square_occupied[state->private->trail_pos[c][a]] == SQOC_TRAIL_TAIL(c))
	  state->square_occupied[state->private->trail_pos[c][a]] = SQOC_VACANT;
	a = (state->private->trail_offset[c]
	     + state->private->trail_size[c] - 1) & (maxq - 1);
	if (state->square_occupied[state->private->trail_pos[c][a]] ==
	    SQOC_TRAIL(c))
	  state->square_occupied[state->private->trail_pos[c][a]] =
	    SQOC_TRAIL_TAIL(c);
      } else
	state->square_occupied[state->private->trail_pos[c][a]] =
	  SQOC_TRAIL_TAIL(c);

      /* If the player has fire_trail on, trigger explosions on
	 the head and the tail of the trail.  */
      if (state->player[c].fire_trail) {
	trigger_explosion (state, lvl, state->private->trail_pos[c][a],
			   EXPLOSION_IMMEDIATE);
	trigger_explosion (state, lvl, state->private->trail_pos[c]
			   [state->private->trail_offset[c]],
			   EXPLOSION_IMMEDIATE);
      }
    }

    d2 = state->player[c].y2 * lvl->square_width + state->player[c].x2;
    if (state->player[c].delay == 0)
      d2 = lvl->square_move[state->player[c].way][d2];
    state->player[c].pos = d2;
    state->player[c].x2 = state->square_coord[d2].x;
    state->player[c].y2 = state->square_coord[d2].y;
    state->player[c].d.h.h = 0;

    if (state->player[c].spec == T_TUNNEL) {
      state->player[c].way = state->player[c].tunnel_way;
      state->player[c].old_way = state->player[c].way;
      state->player[c].spec = 0;
    }

    if (cpuon) {
      if (state->player[c].target < 16) {
	if ((state->player[c].cpu & 2) == 0) {
	  if (state->player[c].behaviour == 1)
	    state->player[c].next_way = ia_goto_nearest_bonus (state, lvl,
							       c);
	  else if (state->player[c].behaviour == 2) {
	    if (state->game_mode == M_KILLEM)
	      state->player[c].next_way = ia_goto_nearest_lemming (state, lvl,
								   c);
	    if (state->game_mode == M_TCASH)
	      state->player[c].next_way = ia_goto_nearest_cash (state, lvl,
								c);
	    if (state->game_mode == M_COLOR)
	      state->player[c].next_way = ia_goto_nearest_color (state, lvl,
								 c);
	  } else
	    state->player[c].next_way =
	      ia_goto_target (state, lvl,
			      c, state->player[state->player[c].target].x2,
			      state->player[state->player[c].target].y2);
	}
      } else
	state->player[c].target -= 16;
    }
    state->square_occupied[d2] = SQOC_VEHICLE_TAIL (c);
    if ((state->square_explo_state[d2] <= EXPLOSION_IMMEDIATE) &&
	state->player[c].invincible == 0)
      state->player[c].spec = 0xff;

    d = (state->player[c].x2 >> 1) +
      (state->player[c].y2 >> 1) * lvl->tile_width;

    if (lvl->square_type[state->player[c].pos] == T_ICE)
      state->player[c].spec = T_ICE;
    if ((lvl->square_type[state->player[c].pos] == T_STOP
	 && state->player[c].delay == 0)
	|| state->player[c].notify_delay) {
      state->player[c].notify_delay = 0;
      state->player[c].delay = 100;
      state->player[c].d.e = 0;
      return;
    }

    if (state->game_mode == M_KILLEM) {
      tmppti = state->private->square_lemmings_list[d2];
      if (tmppti) {
	int lemmings_move_offset = state_lemmings_move_offset (state);
	assert (tmppti >= state->private->lemmings_support
		&& tmppti < state->private->lemmings_support + lemmings_total);
	if ((tmppti->pos_tail == d2 && lemmings_move_offset < 38000)
	    || (tmppti->pos_head == d2 && lemmings_move_offset > 28000)) {
	  if (!state->private->level_is_finished) {
	    state->player[c].score += 10;
	    state->player[tmppti->couleur].lemmings_nbr--;
	  }
	  tmppti->dead = (rand () & 15) + 1;
	  if (rand () & 63) {
	    tmppti->couleur = 0;
	    if (state->player[c].cpu == 2)
	      event_sfx (90 + ((tmppti->dead - 1) >> 1));
	  } else {
	    tmppti->couleur = 1;
	    if (!state->private->level_is_finished)
	      state->player[c].score += 140;
	    state->player[c].martians_nbr++;
	    if (state->player[c].cpu == 2)
	      event_sfx (98);
	  }
	  /* We will assign the dead lemming to the nearest square.  */
	  if (lemmings_move_offset < 32536) {
	    /* If it's the tail square, the offset can be kept as-is.  */
	    tmppti->min = lemmings_move_offset;
	    i = tmppti->pos_tail;
	  } else {
	    /* If it's the head square, the offset and the
	       direction needs to be inverted.  */
	    tmppti->min = 65536 - lemmings_move_offset;
	    assert (tmppti->min < 65536);
	    i = tmppti->pos_head;
	    tmppti->dir = REVERSE_DIR (tmppti->dir);
	  }
	  state->private->square_lemmings_list[tmppti->pos_tail] = NULL;
	  state->private->square_lemmings_list[tmppti->pos_head] = NULL;
	  tmppti->next_dead = state->private->square_dead_lemmings_list[i];
	  state->private->square_dead_lemmings_list[i] = tmppti;
	}
      }
      for (i = 0; i < 4; i++)
	if (state->player[(i + 1) & 3].lemmings_nbr == 0 &&
	    state->player[(i + 2) & 3].lemmings_nbr == 0 &&
	    state->player[(i + 3) & 3].lemmings_nbr == 0)
	  state->private->level_is_finished = i + 1;
    }
    if (state->game_mode == M_COLOR) {
      t = state->square_object[d2];
      if ((signed char) t >= 0) {
	if ((!state->private->level_is_finished)) {
	  state->player[c].score += 2;
	  if (state->player[c].cpu == 2) {
	    if (t == (signed char) c)
	      event_sfx (100);
	    else if (t <= 4)
	      event_sfx (101);
	    else if (t == (signed char)(8 + c))
	      event_sfx (102);
	    else if (t <= 12)
	      event_sfx (103);
	    else if (t == 16)
	      event_sfx (104);
	    else if (t == 24)
	      event_sfx (105);
	  }
	  if ((t < 4) && (state->player[t].spec != 0xde))
	    state->player[t].cash++;
	  else if (t == 4) {
	    if (state->player[(c + 1) & 3].spec != 0xde)
	      state->player[(c + 1) & 3].cash++;
	    if (state->player[(c + 2) & 3].spec != 0xde)
	      state->player[(c + 2) & 3].cash++;
	    if (state->player[(c + 3) & 3].spec != 0xde)
	      state->player[(c + 3) & 3].cash++;
	  } else if (t < 12) {
	    if (state->player[t & 3].cash > 0)
	      state->player[t & 3].cash--;
	  } else if (t == 12) {
	    if ((state->player[(c + 1) & 3].cash > 0)
		&& (state->player[(c + 1) & 3].spec != 0xde))
	      state->player[(c + 1) & 3].cash--;
	    if ((state->player[(c + 2) & 3].cash > 0)
		&& (state->player[(c + 2) & 3].spec != 0xde))
	      state->player[(c + 2) & 3].cash--;
	    if ((state->player[(c + 3) & 3].cash > 0)
		&& (state->player[(c + 3) & 3].spec != 0xde))
	      state->player[(c + 3) & 3].cash--;
	  } else if (t == 16)
	    state->player[c].time += 1000;
	  else if (t == 24) {
	    if (state->player[c].time > 333)
	      state->player[c].time -= 333;
	    else
	      state->player[c].time = 1;
	  }
	}
	state->square_object[d2] = SQOB_NOTHING;
	/* add_color(0); */
	state->private->objects_nbr--;
      }
    }

    if (state->game_mode == M_TCASH) {
      t = state->square_object[d2];
      if ((signed char) t >= 0) {
	if (!state->private->level_is_finished) {
	  state->player[c].score += 2;
	  if (t == 0) {
	    state->player[c].cash++;
	    if (state->player[c].cpu == 2)
	      event_sfx (80);
	  }
	  if (t == 15) {
	    state->player[c].time += 1000;
	    if (state->player[c].cpu == 2)
	      event_sfx (81);
	  }
	}
	state->square_object[d2] = -1;
	/* add_cash(0); */
	state->private->objects_nbr--;
      }
    }

    if (state->game_mode == M_DEATHM) {
      for (i = 0; i < 4; i++)
	if (state->player[(i + 1) & 3].lifes == 0
	    && state->player[(i + 2) & 3].lifes == 0
	    && state->player[(i + 3) & 3].lifes == 0)
	  state->private->level_is_finished = i + 1;
    }
    {
      int bonus = tile_bonus[d];
      if (bonus && bonus != 0xff) {
	rem_bonus (state, lvl, d);
	if (!state->private->level_is_finished) {
	  state->player[c].score += 10;
	  if (bonus & 128) {
	    if (state->player[c].cpu == 2)
	      event_sfx (39 + (bonus & 127));
	    for (i = 0; i < 4; i++)
	      if ((c != i) && (state->player[i].spec != 0xde))
		apply_bonus (state, lvl, i, (bonus & 127));
	  } else
	    apply_bonus (state, lvl, c, bonus);
	}
	if (state->player[c].notify_delay) {
	  state->player[c].notify_delay = 0;
	  state->player[c].delay = 100;
	  state->player[c].d.e = 0;
	  return;
	}
      }
    }
    state->player[c].old_old_way = state->player[c].old_way;
    state->player[c].old_way = state->player[c].way;

    if (state->player[c].autopilot)
      find_free_way (state, lvl, c);
    if ((!state->player[c].autopilot)
	&& (state->player[c].next_way == (state->player[c].old_way ^ 2)))
      state->player[c].next_way = state->player[c].old_way;
    if (state->player[c].spec != T_ICE)
      state->player[c].way = state->player[c].next_way;
    else {
      state->player[c].next_way = state->player[c].way;
      state->player[c].spec = 0;
    }

    idx = lvl->square_move[state->player[c].way][d2];
    if (idx == INVALID_INDEX)
      state->player[c].spec = 0xff;
    else if (state->square_occupied[idx] != SQOC_VACANT)
      state->player[c].spec = 0xff;

    if (state->player[c].spec == 0xff) {
      erase_trail (state, lvl, c);
      if (! state->private->level_is_finished) {
	if (! state->player[c].invincible && state->game_mode != M_DEATHM)
	  shrink_trail (state, c, 5);
	if (state->player[c].lifes == 1) {
	  state->player[c].lifes = 0;
	  state->player[c].spec = 0xde;
	  if (!(((state->player[0].cpu & 2) && (state->player[0].lifes))
		|| ((state->player[1].cpu & 2) && (state->player[1].lifes))
		|| ((state->player[2].cpu & 2) && (state->player[2].lifes))
		|| ((state->player[3].cpu & 2) && (state->player[3].lifes)))) {
	    state->private->level_is_finished = 15;
	  }
	  if (state->player[c].cpu == 2)
	    event_sfx (62);
	  return;
	}
      }
      state_reinit_player (state, lvl, c);
      if (state->player[c].lifes != 0 && state->player[c].invincible == 0
	  && (!state->private->level_is_finished)) state->player[c].lifes--;
      if (!state->private->level_is_finished) {
	if (state->player[c].lifes > 1) {
	  if (state->player[c].cpu == 2)
	    event_sfx (60);
	} else {
	  if (state->player[c].cpu == 2)
	    event_sfx (61);
	}
	{
	  char txt_tmp[128];
	  /* TRANS: %d, the number of remaining lives, is always
	     positive.  */
	  sprintf (txt_tmp, ngettext("LAST LIFE", "%d LIVES LEFT",
				     state->player[c].lifes),
		   state->player[c].lifes);
	  set_txt_bonus (c, txt_tmp, 150);
	}
      }
      state->player[c].invincible = 350;
      return;
    }

/******************/
    if (lvl->square_type[state->player[c].pos] == T_TUNNEL
	&& lvl->square_direction[state->player[c].pos] == state->player[c].next_way) {
      a_dir dir;
      a_square_index dest;
      state->player[c].spec = T_TUNNEL;
      if ((state->player[c].cpu == 2) && (!state->private->level_is_finished))
	event_sfx (69);
      dest = lvl->square_move[state->player[c].next_way][state->player[c].pos];
      if (lvl->square_type[dest] == T_TUNNEL)
	dir = lvl->square_direction[dest] ^ 2;
      else
	dir = lvl->square_direction[state->player[c].pos] ^ 2;
      state->player[c].tunnel_way = dir;
      state->player[c].next_way = state->player[c].tunnel_way;
      if (state->player[c].tunnel_inverse)
	state->player[c].next_way ^= 2;
    }
/*****************/

    /*    if (state->player[c].spec!=t_tunnel*8) */
    {
      if (lvl->square_type[state->player[c].pos] == T_SPEED) {
	a_dir dir = lvl->square_direction[state->player[c].pos];

	if (state->player[c].way == dir)
	  state->player[c].vi = state->player[c].v;
	else if ((state->player[c].way ^ 2) == dir)
	  state->player[c].vi = -(state->player[c].v >> 1);
	else
	  state->player[c].vi = 0;
      } else
	state->player[c].vi = 0;

      if (lvl->square_type[state->player[c].pos] == T_DUST)
	state->player[c].vi = -(state->player[c].v >> 1);
      trigger_possible_explosion (state, lvl, d2);
    }
    state->player[c].delay = 0;

    assert (lvl->square_move[state->player[c].way][d2] != INVALID_INDEX);
    state->square_occupied[lvl->square_move[state->player[c].way][d2]] =
      SQOC_VEHICLE_HEAD (c);
  }
}
