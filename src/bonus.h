/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
|                                                                         |
| This file is part of Heroes.                                            |
|                                                                         |
| Heroes is free software; you can redistribute it and/or modify it under |
| the terms of the GNU General Public License version 2 as published by   |
| the Free Software Foundation.                                           |
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

#ifndef HEROES__BONUS__H
#define HEROES__BONUS__H

#include "pcx.h"
#include "timer.h"
#include "sprite.h"

extern a_timer bonus_anim_htimer;
extern int bonus_anim_offset;

#define N_BONUSES 16
#define N_BONUS_FRAMES 13
extern a_sprite *bonus_rle[2][N_BONUSES][N_BONUS_FRAMES];

/* globaly initialize bonuses */
extern void init_bonuses (void);
extern void uninit_bonuses (void);

/* initialization routine for each level */
extern void render_init_bonus_level (void);

extern void show_txt_bonus (int pl, a_pixel *dest);
extern void set_txt_bonus (int pl, const char *txt, int tempo);
extern void update_player_bonus_vars (int pl);

#endif
