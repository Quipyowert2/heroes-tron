/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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


#ifndef HEROES__STRUCTS__H
#define HEROES__STRUCTS__H

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

#ifdef __HEDIT__

static int square_offset_320[4] = { 0, 12, 320 * 10, 320 * 10 + 12 };
#ifndef __HEDLITE__
static int square_offset[4] = { 0, 12, 384 * 10, 384 * 10 + 12 };
static char d2w[9] = { 0, 0, 1, 1, 2, 2, 2, 2, 3 };
static char w2d[4] = { d_up, d_right, d_down, d_left };
#endif

static char *type_name[] =
  { "NONE", "STOP", "SPEED", "TUNNEL", "BOOM", "ANIM", "ICE", "DUST",
  "OUTWAY"
};

static char spd_test[9][12] = { 
{2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8},
{2, 2, 2, 4, 4, 4, 4, 4, 4, 8, 8, 8},
{2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8},
{2, 2, 2, 2, 2, 4, 4, 8, 8, 8, 8, 8},
{2, 2, 2, 2, 2, 2, 8, 8, 8, 8, 8, 8},
{2, 2, 2, 2, 2, 1, 1, 8, 8, 8, 8, 8},
{2, 2, 2, 2, 1, 1, 1, 1, 8, 8, 8, 8},
{2, 2, 2, 1, 1, 1, 1, 1, 1, 8, 8, 8},
{2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8}
};

static char dir_test[9][12] = { 
{1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3},
{1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3},
{1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3},
{1, 1, 1, 1, 1, 2, 2, 3, 3, 3, 3, 3},
{1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3},
{1, 1, 1, 1, 1, 0, 0, 3, 3, 3, 3, 3},
{1, 1, 1, 1, 0, 0, 0, 0, 3, 3, 3, 3},
{1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 3, 3},
{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3}
};

#endif
/*------------------- tiles structures --------------------*/
typedef struct
{
  unsigned long int output;
  unsigned char direction:4;
  unsigned char tempo:4;
}
__attribute__ ((packed)) tunnel_t;

typedef struct
{
  unsigned char frame_nbr;
  unsigned char speed;		/* in VBL */
}
__attribute__ ((packed)) anim_t;


typedef union
{
  unsigned char param[5];
  tunnel_t tunnel;
  anim_t anim;
}
__attribute__ ((packed)) param_u;

/****************************************/
typedef struct
{
  unsigned long int number;	/* 4 */
  unsigned char collision[4];	/* 4 */
  unsigned short int sprite;	/* 2 */
  param_u info;			/* 5 */
  unsigned char type;		/* 1 */
}
__attribute__ ((packed)) tile_t;	/* size = 16 */
/****************************************/
typedef struct
{
  unsigned char collision[4];	/* 4 */
  param_u info;			/* 5 */
  unsigned char type;		/* 1 */
}
__attribute__ ((packed)) tile_info_t;	/* size = 10 */
/****************************************/


typedef struct
{
  unsigned long int xt;		/* 4 */
  unsigned long int yt;		/* 4 */
  unsigned long int xwrap;	/* 4 */
  unsigned long int ywrap;	/* 4 */
  unsigned long int start[4];	/* 16 */
  unsigned char start_way[4];	/* 4  direction and sub-tile */
  char tile_set_name[9];	/* 9 */
  char soundtrack_name[9];	/* 9 */
  char unused[10];		/* 10 */
}
__attribute__ ((packed)) level_header_t;	/* sum => 64 */

/*------------------ player records -------------------*/

/* these two structures are used to access
   the higher and lower part of a long int */
typedef struct {
#ifdef WORDS_BIGENDIAN
  short int h, l;
#else
  short int l, h;
#endif
} hl;

typedef union {
  long int e;
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
  int way, next_way, old_way, old_old_way, tunnel_way;
  int square;			/* sub-tile */
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
  char way;
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
/*------------------------------ HighScores -------------------------------*/

typedef struct
{
  char name[9];
  unsigned char magic;
  unsigned char unused1;
  char unused2;
  unsigned long int points;
}
__attribute__ ((packed)) top_score;


/*------------------------------ SavedGames -------------------------------*/
typedef struct
{
  char name[16];
  unsigned int level;
  unsigned int points[4];
  unsigned int lifes[4];
  unsigned char magic;
  char used;
}
__attribute__ ((packed)) saved_game;

/*------------------------------ ---------- -------------------------------*/

#endif /* HEROES__STRUCTS__H */
