/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
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
  a_u32 output;
  /* tempo was never used with tunnels.  The original
     purpose was to delay the player underground (for the time
     given by delay, computed by the level editor from the
     length of the tunnel).  Its has been abandoned because
     handling of vehicles "out of the map" would complexify
     the game internals.

     FIXME: If we can make sure that tempo is 0 in
     *all* level files, the two fields below should better
     be replaced by
        a_u8 direction;
  */
#ifdef WORDS_BIGENDIAN
  a_u8 tempo:4;
  a_u8 direction:4;
#else
  a_u8 direction:4;
  a_u8 tempo:4;
#endif
}
ATTRIBUTE_PACKED a_tunnel;

typedef struct
{
  a_u8 frame_nbr;
  a_u8 speed;		/* in VBL */
}
ATTRIBUTE_PACKED an_anim;


typedef union
{
  a_u8 param[5];
  a_tunnel tunnel;
  an_anim anim;
}
ATTRIBUTE_PACKED a_param;

typedef struct
{
  a_u32 number;
  a_u8 collision[4];
  a_u16 sprite;
  a_param info;
  a_u8 type;
}
ATTRIBUTE_PACKED a_tile; /* 16 bytes */

typedef struct
{
  a_u8 collision[4];
  a_param info;
  a_u8 type;
}
ATTRIBUTE_PACKED a_tile_info;  /* 10 bytes */

#define FILENAME_SIZE 8

typedef struct
{
  a_u32 xt;
  a_u32 yt;
  a_u32 xwrap;
  a_u32 ywrap;
  a_u32 start[4];		/* starting tile */
  a_u8 start_way[4];		/* starting direction and square */
  char tile_set_name[FILENAME_SIZE + 1];
  char soundtrack_name[FILENAME_SIZE + 1];
  char unused[10];
}
ATTRIBUTE_PACKED a_level_header; /* 64 bytes */

#endif /* HEROES__STRUCTS__H */
