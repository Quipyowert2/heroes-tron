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

typedef struct
{
  int x, y;			/* position (tile coords) */
  int x2, y2;			/* position (square coords) */
  int pos;			/* posision (square index) */
  int v;			/* speed */
  int vi;			/* additional speed */
  int vitt;			/* speed to reach */
  int vitp;			/* current speed */
  ehl d;			/* offset */
  a_dir way, next_way, old_way, old_old_way, tunnel_way;
  int delay;			/* delay frames (FIXME: don't use frames) */
  int spec;			/* special event (tunnel,ice,death) */
  int div;			/* misc.         (tunnel) */
  int inversed_controls;	/* inverted commands */
  unsigned int score;			/* points */
  unsigned int score_delta;		/* points++ */
  int turbo_level;		/* turbo left */
  int turbo_level_delta;	/* turbo left++ */
  int speedup;			/* bonus speedup or speeddown */
  int rotozoom;			/* roto */
  int rotozoom_direction;
  int waves;
  int waves_begin;
  int fire_trail;
  int invincible;		/* blinking */
  int lifes;			/* lifes LEFT */
  int turbo;
  char notify_delay;		/* a pause is coming */
  char tunnel_inverse;
  char autopilot;
  char cpu;			/* 0: local CPU    [1: remote CPU] */
				/* 2: player local [,3: player distant] */
  int ia_max_depth;		/* recusrion depth for CPU (keep <= 7,
				   or it will be slow) */
  int target;			/* target to follow */
  int lemmings_nbr;
  int martians_nbr;		/* ;-) */
  int time;
  int cash;			/* ... or colors */
  int wins;			/* games won so far */
}
a_player;

/** -- END PUBLIC -- **/
