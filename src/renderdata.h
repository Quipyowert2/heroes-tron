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

/*----------------------------------------------------------.
| This file defines data used by the renderering function.  |
`----------------------------------------------------------*/

#ifndef HEROES__RENDERDATA__H
#define HEROES__RENDERDATA__H

#include "display.h"
#include "pcx.h"

/* data for background tiles */

enum tile_anim_kind { A_NONE, A_LOOP, A_PINGPONG };

typedef struct {
  const pixel_t *source;	/* address of the tile's image */
  enum tile_anim_kind kind;	/* kind of animation */
  int anim_speed;		/* speed for animated tiles */
  int anim_frames;		/* number of frames in an animation */
} bg_data_t;

extern bg_data_t *bg_data;

/* data for foreground sprites (trees...) and bonuses, these are kept
   in the same struct because they are drawn in the same loop */

typedef struct {
  const pixel_t *sprite;	/* transparent sprite if non nil */
  const pixel_t *bonus;		/* bonus line, if non nil */
  char big_dollar;		/* 1 if a big dollar must be drawn */
} fg_data_t;

extern fg_data_t *fg_data;

extern void init_render_data (void);
extern void uninit_render_data (void);

#endif /* HEROES__RENDERDATA__H */
