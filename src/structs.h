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


#ifndef _PIECE_H_plx_
#define _PIECE_H_plx_

/*----------------------- Constantes des directions -----------------------*/
#define d_up   1
#define d_right 2
#define d_down    4
#define d_left 8

#define w_up   0
#define w_right 1
#define w_down    2
#define w_left 3

/*------------------- Constantes des tests de collision -------------------*/
#define c_down    1
#define c_left 2
#define c_up   4
#define c_right 8

/*-------------------- Constantes des types de pièces ---------------------*/
#define type_nbr 9		/* nombre de types */

#define t_none    0
#define t_stop    1
#define t_speed   2		/* infos en: dalemem.info.param[] */
#define t_tunnel  3		/* infos en: dalemem.info.tunnel */
#define t_boom    4
#define t_anim    5		/* infos en: dalemem.info.anim */
#define t_ice     6		/* infos en: dalemem.info.param[] */
#define t_dust    7		/* infos en: dalemem.info.param[] */
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
/*------------------- Structures de stockage de pièces --------------------*/
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
  unsigned char speed;		// in VBL
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
  unsigned long int number;	// 4
  unsigned char collision[4];	// 4
  unsigned short int sprite;	// 2
  param_u info;			// 5
  unsigned char type;		// 1
}
__attribute__ ((packed)) tile_t;	// size = 16
/****************************************/
typedef struct
{
  unsigned char collision[4];	// 4
  param_u info;			// 5
  unsigned char type;		// 1
}
__attribute__ ((packed)) tile_info_t;	// size = 10
/****************************************/


typedef struct
{
  unsigned long int xt;		// 4
  unsigned long int yt;		// 4
  unsigned long int xwrap;	// 4
  unsigned long int ywrap;	// 4
  unsigned long int start[4];	// 16
  unsigned char start_way[4];	// 4 // way et sous-case
  char tile_set_name[9];	// 9
  char soundtrack_name[9];	// 9
  char unused[10];		// 10
}
__attribute__ ((packed)) level_header_t;	// þ=> 64

	/*
	   typedef union
	   {
	   long int d;
	   signed char square[4];
	   } docc;
	 */

/*------------------ Structures de stockage des joueurs -------------------*/

typedef struct
{
  short int l, h;
}
hl;
typedef union
{
  long int e;
  hl h;
}
ehl;

typedef struct
{
  int x, y;			// position
  int x2, y2;			// position square_occupied;
  int pos;			// adresse case
  int v;			// speed
  int vi;			// speed additionnelle
  int vitt;			// speed à atteindre
  int vitp;			// speed courrante
  ehl d;			// deplacement
  int way, next_way, old_way, old_old_way, tunnel_way;
  int square;			// souscase
  int delay;			// frames d'attente
  int spec;			// evenement special (tunnel,ice,mort)
  int div;			// divers            (tunnel)
  int inversed_controls;	// inverse commande
  unsigned int score;			// points
  unsigned int score_delta;		// points++
  int turbo_level;		// turbo restant
  int turbo_level_delta;	// turbo restant++
  int speedup;			// bonus speedup ou speeddown
  int rotozoom;			// roto
  int rotozoom_direction;
  int waves;			// gel
  int waves_begin;
  int invincible;		// clignotant
  int lifes;			// nbr de lifes RESTANTES
  int turbo;
  char notify_delay;		// une pause lui pend sous le nez
  char tunnel_inverse;
  char autopilot;
  char cpu;			// 0: CPU local    [,1: CPU distant]   ,
  // 2: player local [,3: player distant]
  int ia_max_depth;		// profondeur de vision pour CPU (pas plus de 7)
  // (à 8 ça saccade assez)
  int behaviour;		// 0. fillature 1.miambonus 2.ecrase 3...
  int target;			// pour jouer au chat et au mulot
  int lemmings_nbr;		// et oui!
  int martians_nbr;		// ;-)
  int time;
  int cash;			// ou points de couleur
  int wins;			// parties gagnées
}
player_t;
typedef struct
{
  unsigned int pos1, pos2;	// positions
  unsigned int min;		// coordonées dans la case
  char *nexttache;		// tache suivante de la case
  char way;
  int couleur;
  char dead;
}
lemming_t;
/*----------------------- Differents modes de jeu -------------------------*/
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
//          char              questmode;
  unsigned char magic;
  char used;
}
__attribute__ ((packed)) saved_game;

/*------------------------------ ---------- -------------------------------*/

#endif
