/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License as published by |
| the Free Software Foundation; either version 2 of the License, or |
| (at your option) any later version.                               |
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

#ifndef HEROES__STRUCTS__H
#define HEROES__STRUCTS__H

#include "lvl.h"

/*----------------------- directions constants -----------------------*/
#define d_up   1
#define d_right 2
#define d_down    4
#define d_left 8

#define w_up   0
#define w_right 1
#define w_down    2
#define w_left 3

/*------------------- collide test constants -------------------*/
#define c_down    1
#define c_left 2
#define c_up   4
#define c_right 8

/*-------------------- tile types ---------------------*/
#define type_nbr 9		/* number of types */

#define t_none    0
#define t_stop    1
#define t_speed   2		/* info in: dalemem.info.param[] */
#define t_tunnel  3		/* info in: dalemem.info.tunnel */
#define t_boom    4
#define t_anim    5		/* info in: dalemem.info.anim */
#define t_ice     6		/* info in: dalemem.info.param[] */
#define t_dust    7		/* info in: dalemem.info.param[] */
#define t_outway  8

/*------------------- tiles structures --------------------*/
typedef struct
{
  u32_t output;
  /* tempo was never used with tunnels.  The original
     purpose was to delay the player underground (for the time
     given by delay, computed by the level editor from the
     length of the tunnel).  Its has been abandoned because
     handling of vehicles "out of the map" would complexify
     the game internals.

     FIXME: If we can make sure that tempo is 0 in
     *all* level files, the two fields below should better
     be replaced by
        u8_t direction;
  */
#ifdef WORDS_BIGENDIAN
  u8_t tempo:4;
  u8_t direction:4;
#else
  u8_t direction:4;
  u8_t tempo:4;
#endif
}
ATTRIBUTE_PACKED tunnel_t;

typedef struct
{
  u8_t frame_nbr;
  u8_t speed;		/* in VBL */
}
ATTRIBUTE_PACKED anim_t;


typedef union
{
  u8_t param[5];
  tunnel_t tunnel;
  anim_t anim;
}
ATTRIBUTE_PACKED param_t;

typedef struct
{
  u32_t number;
  u8_t collision[4];
  u16_t sprite;
  param_t info;
  u8_t type;
}
ATTRIBUTE_PACKED tile_t; /* 16 bytes */

typedef struct
{
  u8_t collision[4];
  param_t info;
  u8_t type;
}
ATTRIBUTE_PACKED tile_info_t;  /* 10 bytes */

#define FILENAME_SIZE 8

typedef struct
{
  u32_t xt;
  u32_t yt;
  u32_t xwrap;
  u32_t ywrap;
  u32_t start[4];		/* starting tile */
  u8_t start_way[4];		/* starting direction and square */
  char tile_set_name[FILENAME_SIZE + 1];
  char soundtrack_name[FILENAME_SIZE + 1];
  char unused[10];
}
ATTRIBUTE_PACKED level_header_t; /* 64 bytes */

/*------------------ player records -------------------*/

/* these two structures are used to access
   the higher and lower part of a u32_t */
typedef struct {
#ifdef WORDS_BIGENDIAN
  u16_t h, l;
#else
  u16_t l, h;
#endif
} hl;

typedef union {
  u32_t e;
  hl h;
} ehl;

typedef struct
{
  int x, y;			/* position */
  int x2, y2;			/* position square_occupied; */
  int pos;			/* address square */
  int v;			/* speed */
  int vi;			/* additional speed */
  int vitt;			/* speed to reach */
  int vitp;			/* current speed */
  ehl d;			/* offset */
  dir_t way, next_way, old_way, old_old_way, tunnel_way;
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
  int behaviour;		/* 0.follower 1.bonus eater 2.squisher 3...*/
  int target;			/* target to follow */
  int lemmings_nbr;
  int martians_nbr;		/* ;-) */
  int time;
  int cash;			/* ... or colors */
  int wins;			/* games win */
}
player_t;
typedef struct
{
  unsigned int pos1, pos2;	/* positions */
  unsigned int min;		/* position in the tile */
  char *nexttache;		/* next stain in the tile */
  dir_t way;
  int couleur;
  char dead;
}
lemming_t;
/*----------------------- game modes -------------------------*/
#define M_QUEST  0
#define M_DEATHM 1
#define M_KILLEM 2
#define M_TCASH  3
#define M_COLOR  4


#endif /* HEROES__STRUCTS__H */
