/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
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

#ifndef HEROES__SPRITES__H
#define HEROES__SPRITES__H

/*-------------------------------------------------------------------.
| This file define an abstraction for sprites, i.e., small blocks of |
| pixels that can be pasted in memory (but you knew that).           |
`-------------------------------------------------------------------*/

#include "display.h"

/* Different kind of sprites, they are not all used yet.  These values
   should be used *internally* by the sprite handling functions.  */
enum sprite_kind {
  //  S_OPAQUE,			/* opaque (yes!) */
  S_RLE,			/* transparant */
  //  S_RLE_GL,			/* transparant, using a line of glenz */
  //  S_RLE_ZCOL, 		/* transparant, using a kind of
  //				   color-based z-buffer */
  S_PROG,			/* a list of sprites, to draw all at once */
  S_PROG_WAV			/* like S_PROG but also wave */
};

typedef union sprite_s sprite_t;

typedef void (*sprite_draw_func) (const sprite_t *sprite, pixel_t *dest);

/*-------------------------------------------------------------------.
| Because the various flavor of sprites are not drawn the same way,  |
| there exist various drawing functions.  There is one drawing       |
| function for each kind of sprite.                                  |
|                                                                    |
| Each sprite contains a pointer to its drawing function.  Therefore |
| the DRAW_SPRITE macro below is the generic way to draw a sprite.   |
| It is more efficient, however, to call the right drawing function  |
| if you know it at compile time.                                    |
`-------------------------------------------------------------------*/

#define DRAW_SPRITE(s,dest) ((s)->draw ((s), (dest)))

#define SPRITE_FIRST_MEMBER			\
  sprite_draw_func	draw


#define SPRITE_COMMON_MEMBERS				\
  SPRITE_FIRST_MEMBER;					\
  enum sprite_kind	kind	/* kind of sprite */

struct sprite_common_s {
  SPRITE_COMMON_MEMBERS;
};

typedef struct sprite_prog_list_s sprite_prog_list_t; /* see sprprog.c */

struct sprite_prog_s {
  SPRITE_COMMON_MEMBERS;
  sprite_prog_list_t *list;
};

struct sprite_rle_s {
  SPRITE_COMMON_MEMBERS;

  /* The code of an rle-sprite is a sequence of
       1 u8_t: number m of transparent pixels to skip
       1 u8_t: number n of bytes to write
       n u8_t: actual bytes to write
     The end of a line can be announced using m=0 and n=0,
     and the code MUST terminate by m=0 and n=0.
  */
  u8_t*		code;

  /* a pointer the byte right after the end of code,
     in order to known where to stop. */
  u8_t*		end_code;

  /* the number of byte to skip to from the end of a line to the
     beginning of the next one */
  unsigned int	line_skip;
};

union sprite_s {
  SPRITE_FIRST_MEMBER;
  struct sprite_common_s	all;
  struct sprite_prog_s		prog;
  struct sprite_rle_s		rle;
};

/* generic sprite freeing function, this will dispatch to the right
   freeing function */
void free_sprite (sprite_t *sprite);

#define FREE_SPRITE0(x)				\
  do {						\
    if (x) {					\
      free_sprite (x);				\
      x = 0;					\
    }						\
  } while (0)

#endif /* HEROES__SPRITES__H */
