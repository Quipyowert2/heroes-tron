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

#ifndef HEROES__CONST__H
#define HEROES__CONST__H

/* this file carries too many unrelated stuffs */

#include "pcx.h"
#include "structs.h"
#include "gameid.h"
#include "lvl.h"
#include "state.h"

extern int rounds_nbr_values[16];

extern a_pixel *(render_buffer[2]);	/* xbuf * ybuf */

char key_or_joy_ready (void);
a_keycode get_key_or_joy (void);

extern char kbjoy[6];
extern char kbjoyold[6];
extern char in_jokebox;
extern char in_menu;
extern char in_demo;

extern char demo_ready;

extern a_pixel glenz[8][256];
void draw_glenz_box (a_pixel *dest, int c, int xt, int yt);

extern a_pcx_image main_font_img, vehicles_img;
extern a_pcx_image bonus_font_img;
extern a_pcx_image tile_set_img, font_deck_img;

extern signed char minisinus[32];
extern bool two_players;

extern a_level lvl;
extern a_level_state state;

extern a_gameid game_id;

#endif /* HEROES__CONST__H */
