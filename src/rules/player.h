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

#ifndef HEROES__PLAYER__H
#define HEROES__PLAYER__H

/** -- BEGIN PUBLIC -- **/

/* these two structures are used to access
   the higher and lower part of a a_u32 */
typedef struct {
#ifdef WORDS_BIGENDIAN
  a_u16 h, l;
#else
  a_u16 l, h;
#endif
} hl;

typedef union {
  a_u32 e;
  hl h;
} ehl;

/* FIXME: There are meny fields here that should not be public.  */

typedef struct a_player a_player;
struct a_player
{
  /* Position, in various format.  */
  a_tile_coord tx, ty;
  a_square_coord sx, sy;
  a_square_index si;

  int vitp;			/* current speed */
  ehl d;			/* offset */

  a_dir way;			/* Actual heading direction.  */
  a_dir old_way;		/* Previous one (i.e. was the vehicle
				   was on the previous square; it may be the
				   same.  */
  a_dir tunnel_way;		/* After running throught a tunnel, the
				   vehicle might get out with another heading
				   direction (tunnel_way) than on entrance
				   (way).  */
  a_dir next_way;		/* The direction that shall be taken on next
				   square, if possible.  */

  int turbo;			/* Throttle. */

  int inversed_controls;	/* inverted commands */

  unsigned int score;		/* The present score.  */
  unsigned int score_delta;	/* A value stepping towards "score", for
				   the renderer (FIXME: this ought to
				   belong to the renderer data).  */

  int turbo_level;		/* The present value for the turbo gauge. */
  int turbo_level_delta;	/* A value stepping toward "turbo_level,
				   for the renderer (FIXME: this ought
				   to belong to the renderer data).  */

  int delay;			/* delay frames (FIXME: don't use frames) */
  int speedup;			/* bonus speedup or speeddown */
  int rotozoom;			/* roto */
  int rotozoom_direction;
  int waves;
  int waves_begin;
  int fire_trail;
  int invincible;		/* blinking */

  int lifes;			/* lifes LEFT */
  char notify_delay;		/* a pause is coming */
  char tunnel_inverse;
  char autopilot;
  char cpu;			/* 0: local CPU    [1: remote CPU] */
				/* 2: player local [,3: player distant] */
  int lemmings_nbr;
  int martians_nbr;		/* ;-) */
  int time;
  int cash;			/* ... or colors */
  int wins;			/* games won so far */

  /* FIXME: Get rid of these horrors.  */
  int spec;			/* special event (tunnel,ice,death) */
  int ai_max_depth;		/* recusrion depth for CPU (keep <= 7,
				   or it will be slow) */
  int target;			/* target to follow */

};

/** -- END PUBLIC -- **/

typedef struct a_player_internal a_player_internal;
struct a_player_internal {
  int v;			/* base speed, as configured in opt.speed */
  int vi;			/* additional speed (T_DUST and T_SPEED)*/
  int vitt;			/* speed to reach with account for throttle */

  a_dir old_old_way;		/* The direction previous to old_way.
				   Apparently this is only used by
				   the autopilot.  */
};

#endif /* HEROES__PLAYER__H */
