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

#include "bonus.h"		/* FIXME: Get rid of this.  */
#include "explosions.h"		/* FIXME: Get rif of this.  */
#include "sfx.h"		/* FIXME: Get rif of this.  */
#include "argv.h"		/* FIXME: Get rif of this.  */


/* search for a free direction */
static void
find_free_way (a_level_state *state, int c)
{
  const a_level *lvl = state->level;
  int d = 0, n = 0, o[4] = { 0xff, 0xff, 0xff, 0xff }, e;
  int i, m;
  a_dir f;
  a_player *const p = state->player[c];
  a_level_state_bits *const bits = state->private;

  m = p->sx + p->sy * lvl->square_width;
  e = 1;
  for (i = 0; i < 4; i++) {
    a_square_index idx = lvl->square_move[i][m];
    if (idx != INVALID_INDEX)
      o[i] = state->square_occupied[idx];

    /* Forbid turn back.  This is usually not needed because the
       square behind the vehicle is already occupied, but in some
       tunnel configurations this may not be the case. */
    o[REVERSE_DIR (p->way)] = c;

    if (o[i] != 0xff || idx == INVALID_INDEX)
      d |= e;
    e += e;
  }

  f = p->next_way;

  /* Since the auto pilot is deactivated in case a player runs into a
     fire trail, it's possible for a human player to do a one-eighty
     turn and crash into his own tail. That can be quite annoying in
     case you want to go back one square left or right of your current
     lane and you're too fast pressing the buttons.

     Explicitly ignore the new direction in this case.  */
  if (f == REVERSE_DIR (p->way))
    p->next_way = p->way;

  /* If the way is free the autopilot has nothing to do.  */
  if (!(d & (1 << f)))
    return;

  /* If we reach this place, NEXT_WAY cannot be taken because there is
     a wall or someone else.  Therefore we will want to find some
     other direction automatically.  */

  if (p->cpu & 2) {
    /* The autopilot for human players does not work against fire trails,
       that would be too easy :).
       If NEXT_WAY would lead to a fired square, return immediately,
       unless the player is invincible, in which case the autopilot still
       apply.  */
    a_square_index idx = lvl->square_move[f][m];
    if (idx != INVALID_INDEX
	&& state->square_explo_state[idx] <= EXPLOSION_IMMEDIATE
	&& !p->invincible)
      return;
  }

  e = o[p->next_way];

  /* when a trail force someone to turn, the owner of this trail is credited */
  if ((e & 3) != c && (!bits->level_is_finished) && e != 0xff
      && (bits->player[e & 3].spec != 0xde))
    bits->player[e & 3].score += 5;

  if (!(d & (1 << bits->iplayer[c].old_old_way))) {
    p->next_way = bits->iplayer[c].old_old_way;
    return;
  }

  if (!(d & (1 << p->old_way))) {
    p->next_way = p->old_way;
    return;
  }

  if (p->spec != T_ICE)
    for (i = 1; i != 16; i += i)
      if (!(d & i))
	n++;

  e = d;
  if (n != 0) {
    n = (char) (1 + rand () % n);
    for (i = 0; n != 0; i++, d >>= 1)
      if (!(d & 1))
	n--;
    p->next_way = (char) (i - 1);
/*  if (w2d[i-1]&e) fatal_error("find_free_way() return nonsense !"); */
    assert (((1 << (i - 1)) & e) == 0);
  } else
    p->spec = 0xff;
}


static void
opponent_action_prepare (const a_player *p, an_opponent_action *opp)
{
  opp->dir = p->next_way;
  switch (p->turbo) {
  case 0:
    opp->throttle = TH_BRAKE;
    break;
  case 1:
    opp->throttle = TH_NORMAL;
    break;
  case 2:
    opp->throttle = TH_SPEEDUP;
    break;
  default:
    abort();
  }
}

static void
opponent_action_honor (a_player *p, const an_opponent_action *opp)
{
  p->next_way = opp->dir;
  switch (opp->throttle) {
  case TH_BRAKE:
    p->turbo = 0;
    break;
  case TH_NORMAL:
    p->turbo = 1;
    break;
  case TH_SPEEDUP:
    p->turbo = 2;
    break;
  }
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
update_player (a_level_state *state, unsigned c)
{
  const a_level *const lvl = state->level;
  a_level_state_bits *bits = state->private;
  a_player *const p = state->player[c];
  a_player_internal *const ip = &state->private->iplayer[c];
  a_square_index idx;
  a_tile_index d;
  int l;
  unsigned i;
  a_square_index d2;
  int t;
  a_lemming *tmppti;

  if ((p->score_delta >> 2) < p->score) {
    ++p->score_delta;
    /* 1 life every 10.000 points */
    if (p->score_delta % (10000 << 2) == 0)
      apply_bonus (state, c, B_EXTRA_LIFE);
  }

/* if ((p->score_delta>>2)>p->score) p->score_delta--; */
  if (p->turbo_level_delta < p->turbo_level) {
    p->turbo_level_delta += 8;
    if (p->turbo_level_delta > p->turbo_level)
      p->turbo_level_delta = p->turbo_level;
  } else if (p->turbo_level_delta > p->turbo_level) {
    p->turbo_level_delta -= 8;
    if (p->turbo_level_delta < p->turbo_level)
      p->turbo_level_delta = p->turbo_level;
  }

  if (p->invincible > 0)
    p->invincible--;
  if (state->game_mode >= M_TCASH) {
    if (p->time > 0) {
      p->time--;
/* stop the game if the player is alone */
/*       if ((!level_is_finished) && */
/*           (bits->player[(c+1)&3].spec==0xde) && */
/*           (bits->player[(c+2)&3].spec==0xde) && */
/*           (bits->player[(c+3)&3].spec==0xde)) { level_is_finished=c+1; return; } */
    } else if (!state->private->level_is_finished) {
      p->spec = 0xde;
      erase_trail (state, c);
      /* stop the game if all human players are dead or
	 if there is no more colors or dollars */
      if ((!(((bits->player[0].cpu & 2) && (bits->player[0].time))
	     || ((bits->player[1].cpu & 2) && (bits->player[1].time))
	     || ((bits->player[2].cpu & 2) && (bits->player[2].time))
	     || ((bits->player[3].cpu & 2) && (bits->player[3].time))))
	  || (state->private->objects_nbr == 0)) {
	/* KLUGE: mark all players whose time is 0 as dead.  This is
	   needed because many players can reach 0 simultaneously but
	   this block is only run for the first player when this is
	   discovered.  */
	for (i = 0; i < 4; ++i)
	  if (bits->player[i].time == 0) {
	    bits->player[i].spec = 0xde;
	    erase_trail (state, i);
	  }
	/* find out the richest player and set level_is_finished accordingly */
	state->private->level_is_finished = 0;
	for (i = 1; i < 4; i++)
	  if (bits->player[i].cash > bits->player[state->private->level_is_finished].cash)
	    state->private->level_is_finished = i;
	++state->private->level_is_finished;
      }
    }
  }
  update_player_bonus_vars (c);
  d = (p->sx >> 1) + (p->sy >> 1) * lvl->tile_width;
  if (p->rotozoom != 0)
    p->rotozoom--;
  if (p->waves != 0) {
    p->waves--;
    if (p->waves > 128 && p->waves_begin < 128)
      p->waves_begin++;
    if (p->waves < 128 && p->waves_begin > 0)
      p->waves_begin--;
  }
  if (p->fire_trail)
    --p->fire_trail;

  if (p->spec == 0xde)
    return;

  if (p->inversed_controls > 0) {
    char txt_tmp[128];
    p->inversed_controls--;
    sprintf (txt_tmp, _("INVERTED %d"), p->inversed_controls / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }

  if (p->delay == 0) {
    if (cpuon && p->target < 16) {
      an_opponent_sig *op = state->private->opponent[c];
      if (op && op->frame_update) {
	an_opponent_action act;
	opponent_action_prepare (state->player[c], &act);
	op->frame_update (state, c, &act, state->private->opponent_data[c]);
	opponent_action_honor (state->player[c], &act);
      }
    }

    if (p->turbo != 1 && p->turbo_level > 0
	&& p->speedup == 0) {
      ip->vitt = (ip->v + ip->vi) * p->turbo;
      p->turbo_level -= 2;
    } else if (p->speedup > 0) {
      ip->vitt = (ip->v + ip->vi) << 1;
      p->speedup--;
    } else if (p->speedup < 0) {
      ip->vitt = (ip->v + ip->vi) >> 1;
      p->speedup++;
    } else
      ip->vitt = (ip->v + ip->vi);
    if (p->vitp < ip->vitt)
      if (p->vitp + 512 < ip->vitt)
	p->vitp += 512;
      else
	p->vitp = ip->vitt;
    else if (p->vitp > ip->vitt) {
      if (p->vitp - 512 > ip->vitt)
	p->vitp -= 512;
      else
	p->vitp = ip->vitt;
    }
    p->d.e += p->vitp;
  } else {
    char txt_tmp[128];
    p->delay--;
    sprintf (txt_tmp, _("STOPPED %d"), p->delay / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }


  if (p->d.h.h != 0 || p->delay == 1) {

/**** handling of trails ****/
    if (p->delay == 0) {
      int a;
      l = p->sx + p->sy * lvl->square_width;
      state->square_occupied[l] = SQOC_TRAIL(c);
      state->private->trail_offset[c] =
	(state->private->trail_offset[c] - 1) & (maxq - 1);
      state->private->trail_pos[c][state->private->trail_offset[c]] = l;
      state->private->trail_way[c][state->private->trail_offset[c]] =
	state->square_way[l] =
	DIR8_PAIR (p->way, p->old_way);
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
      if (p->fire_trail) {
	trigger_explosion (state, state->private->trail_pos[c][a],
			   EXPLOSION_IMMEDIATE);
	trigger_explosion (state, state->private->trail_pos[c]
			   [state->private->trail_offset[c]],
			   EXPLOSION_IMMEDIATE);
      }
    }

    d2 = p->sy * lvl->square_width + p->sx;
    if (p->delay == 0)
      d2 = lvl->square_move[p->way][d2];
    p->si = d2;
    p->sx = state->square_coord[d2].x;
    p->sy = state->square_coord[d2].y;
    p->d.h.h = 0;

    if (p->spec == T_TUNNEL) {
      p->way = p->tunnel_way;
      p->old_way = p->way;
      p->spec = 0;
    }

    if (cpuon) {
      if (p->target < 16) {
	an_opponent_sig *op;
	op = state->private->opponent[c];
	if (op && op->square_update) {
	  an_opponent_action acc;
	  opponent_action_prepare (state->player[c], &acc);
	  op->square_update (state, c, &acc, state->private->opponent_data[c]);
	  opponent_action_honor (state->player[c], &acc);
	}
      } else
	p->target -= 16;
    }
    state->square_occupied[d2] = SQOC_VEHICLE_TAIL (c);
    if ((state->square_explo_state[d2] <= EXPLOSION_IMMEDIATE) &&
	p->invincible == 0)
      p->spec = 0xff;

    d = (p->sx >> 1) +
      (p->sy >> 1) * lvl->tile_width;

    if (lvl->square_type[p->si] == T_ICE)
      p->spec = T_ICE;
    if ((lvl->square_type[p->si] == T_STOP
	 && p->delay == 0)
	|| p->notify_delay) {
      p->notify_delay = 0;
      p->delay = 100;
      p->d.e = 0;
      return;
    }

    if (state->game_mode == M_KILLEM) {
      tmppti = state->square_lemmings_list[d2];
      if (tmppti) {
	int lemmings_move_offset = state_lemmings_move_offset (state);
	assert (tmppti >= state->private->lemmings_support
		&& tmppti < state->private->lemmings_support + LEMMINGS_TOTAL);
	if ((tmppti->pos_tail == d2 && lemmings_move_offset < 38000)
	    || (tmppti->pos_head == d2 && lemmings_move_offset > 28000)) {
	  if (!state->private->level_is_finished) {
	    p->score += 10;
	    bits->player[tmppti->color].lemmings_nbr--;
	  }
	  tmppti->dead = (rand () & 15) + 1;
	  if (rand () & 63) {
	    tmppti->color = 0;
	    if (p->cpu == 2)
	      event_sfx (90 + ((tmppti->dead - 1) >> 1));
	  } else {
	    tmppti->color = 1;
	    if (!state->private->level_is_finished)
	      p->score += 140;
	    p->martians_nbr++;
	    if (p->cpu == 2)
	      event_sfx (98);
	  }
	  /* We will assign the dead lemming to the nearest square.  */
	  if (lemmings_move_offset < 32536) {
	    /* If it's the tail square, the offset can be kept as-is.  */
	    tmppti->puddle_offset = lemmings_move_offset;
	    i = tmppti->pos_tail;
	  } else {
	    /* If it's the head square, the offset and the
	       direction needs to be inverted.  */
	    tmppti->puddle_offset = 65536 - lemmings_move_offset;
	    assert (tmppti->puddle_offset < 65536);
	    i = tmppti->pos_head;
	    tmppti->dir = REVERSE_DIR (tmppti->dir);
	  }
	  state->square_lemmings_list[tmppti->pos_tail] = NULL;
	  state->square_lemmings_list[tmppti->pos_head] = NULL;
	  tmppti->next_puddle = state->square_dead_lemmings_list[i];
	  state->square_dead_lemmings_list[i] = tmppti;
	}
      }
      for (i = 0; i < 4; i++)
	if (bits->player[(i + 1) & 3].lemmings_nbr == 0 &&
	    bits->player[(i + 2) & 3].lemmings_nbr == 0 &&
	    bits->player[(i + 3) & 3].lemmings_nbr == 0)
	  state->private->level_is_finished = i + 1;
    }
    if (state->game_mode == M_COLOR) {
      t = state->square_object[d2];
      if ((signed char) t >= 0) {
	if ((!state->private->level_is_finished)) {
	  p->score += 2;
	  if (p->cpu == 2) {
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
	  if ((t < 4) && (bits->player[t].spec != 0xde))
	    bits->player[t].cash++;
	  else if (t == 4) {
	    if (bits->player[(c + 1) & 3].spec != 0xde)
	      bits->player[(c + 1) & 3].cash++;
	    if (bits->player[(c + 2) & 3].spec != 0xde)
	      bits->player[(c + 2) & 3].cash++;
	    if (bits->player[(c + 3) & 3].spec != 0xde)
	      bits->player[(c + 3) & 3].cash++;
	  } else if (t < 12) {
	    if (bits->player[t & 3].cash > 0)
	      bits->player[t & 3].cash--;
	  } else if (t == 12) {
	    if ((bits->player[(c + 1) & 3].cash > 0)
		&& (bits->player[(c + 1) & 3].spec != 0xde))
	      bits->player[(c + 1) & 3].cash--;
	    if ((bits->player[(c + 2) & 3].cash > 0)
		&& (bits->player[(c + 2) & 3].spec != 0xde))
	      bits->player[(c + 2) & 3].cash--;
	    if ((bits->player[(c + 3) & 3].cash > 0)
		&& (bits->player[(c + 3) & 3].spec != 0xde))
	      bits->player[(c + 3) & 3].cash--;
	  } else if (t == 16)
	    p->time += 1000;
	  else if (t == 24) {
	    if (p->time > 333)
	      p->time -= 333;
	    else
	      p->time = 1;
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
	  p->score += 2;
	  if (t == 0) {
	    p->cash++;
	    if (p->cpu == 2)
	      event_sfx (80);
	  }
	  if (t == 15) {
	    p->time += 1000;
	    if (p->cpu == 2)
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
	if (bits->player[(i + 1) & 3].lifes == 0
	    && bits->player[(i + 2) & 3].lifes == 0
	    && bits->player[(i + 3) & 3].lifes == 0)
	  state->private->level_is_finished = i + 1;
    }
    {
      a_bonus bonus = state->tile_bonus[d];
      if (BONUS_P (bonus)) {
	rem_bonus (state, d);
	if (!state->private->level_is_finished) {
	  p->score += 10;
	  if (BONUS_YELLOW_P (bonus)) {
	    if (p->cpu == 2)
	      event_sfx (39 + BONUS_TYPE (bonus));
	    for (i = 0; i < 4; i++)
	      if ((c != i) && (bits->player[i].spec != 0xde))
		apply_bonus (state, i, BONUS_TYPE (bonus));
	  } else
	    apply_bonus (state, c, bonus);
	}
	if (p->notify_delay) {
	  p->notify_delay = 0;
	  p->delay = 100;
	  p->d.e = 0;
	  return;
	}
      }
    }
    ip->old_old_way = p->old_way;
    p->old_way = p->way;

    if (p->autopilot)
      find_free_way (state, c);
    if ((!p->autopilot)
	&& (p->next_way == (p->old_way ^ 2)))
      p->next_way = p->old_way;
    if (p->spec != T_ICE)
      p->way = p->next_way;
    else {
      p->next_way = p->way;
      p->spec = 0;
    }

    idx = lvl->square_move[p->way][d2];
    if (idx == INVALID_INDEX)
      p->spec = 0xff;
    else if (state->square_occupied[idx] != SQOC_VACANT)
      p->spec = 0xff;

    if (p->spec == 0xff) {
      erase_trail (state, c);
      if (! state->private->level_is_finished) {
	if (! p->invincible && state->game_mode != M_DEATHM)
	  shrink_trail (state, c, 5);
	if (p->lifes == 1) {
	  p->lifes = 0;
	  p->spec = 0xde;
	  if (!(((bits->player[0].cpu & 2) && (bits->player[0].lifes))
		|| ((bits->player[1].cpu & 2) && (bits->player[1].lifes))
		|| ((bits->player[2].cpu & 2) && (bits->player[2].lifes))
		|| ((bits->player[3].cpu & 2) && (bits->player[3].lifes)))) {
	    state->private->level_is_finished = 15;
	  }
	  if (p->cpu == 2)
	    event_sfx (62);
	  return;
	}
      }
      state_reinit_player (state, c);
      if (p->lifes != 0 && p->invincible == 0
	  && (!state->private->level_is_finished)) p->lifes--;
      if (!state->private->level_is_finished) {
	if (p->lifes > 1) {
	  if (p->cpu == 2)
	    event_sfx (60);
	} else {
	  if (p->cpu == 2)
	    event_sfx (61);
	}
	{
	  char txt_tmp[128];
	  /* TRANS: %d, the number of remaining lives, is always
	     positive.  */
	  sprintf (txt_tmp, ngettext("LAST LIFE", "%d LIVES LEFT",
				     p->lifes),
		   p->lifes);
	  set_txt_bonus (c, txt_tmp, 150);
	}
      }
      p->invincible = 350;
      return;
    }

/******************/
    if (lvl->square_type[p->si] == T_TUNNEL
	&& lvl->square_direction[p->si] == p->next_way) {
      a_dir dir;
      a_square_index dest;
      p->spec = T_TUNNEL;
      if ((p->cpu == 2) && (!state->private->level_is_finished))
	event_sfx (69);
      dest = lvl->square_move[p->next_way][p->si];
      if (lvl->square_type[dest] == T_TUNNEL)
	dir = lvl->square_direction[dest] ^ 2;
      else
	dir = lvl->square_direction[p->si] ^ 2;
      p->tunnel_way = dir;
      p->next_way = p->tunnel_way;
      if (p->tunnel_inverse)
	p->next_way ^= 2;
    }
/*****************/

    /*    if (p->spec!=t_tunnel*8) */
    {
      if (lvl->square_type[p->si] == T_SPEED) {
	a_dir dir = lvl->square_direction[p->si];

	if (p->way == dir)
	  ip->vi = ip->v;
	else if ((p->way ^ 2) == dir)
	  ip->vi = -(ip->v >> 1);
	else
	  ip->vi = 0;
      } else
	ip->vi = 0;

      if (lvl->square_type[p->si] == T_DUST)
	ip->vi = -(ip->v >> 1);
      trigger_possible_explosion (state, d2);
    }
    p->delay = 0;

    assert (lvl->square_move[p->way][d2] != INVALID_INDEX);
    state->square_occupied[lvl->square_move[p->way][d2]] =
      SQOC_VEHICLE_HEAD (c);
  }
}

int
state_update (a_level_state *state)
{
  int n = 0;
  long frames = read_htimer (state->private->update_timer);

  update_explosions (state);

  for (; frames; --frames) {
    if (state->private->players_started) {
      update_player (state, 0);
      update_player (state, 1);
      update_player (state, 2);
      update_player (state, 3);
      update_bonuses (state);
    }
    if (state->game_mode == M_KILLEM)
      update_lemmings (state);
    n++;
  }

  return (n);
}


void
state_start_game (a_level_state *state)
{
  reset_htimer (state->private->update_timer);
}

void
state_start_players (a_level_state *state)
{
  state->private->players_started = true;
}

void
state_pause (a_level_state *state, const a_timer pause_timer)
{
  shift_htimer (state->private->update_timer, pause_timer);
}
