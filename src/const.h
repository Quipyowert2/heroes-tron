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

#ifndef HEROES__CONST__H
#define HEROES__CONST__H

/* this file carries too many unrelated stuffs */

#include "pcx.h"
#include "structs.h"
#include "gameid.h"
#include "lvl.h"

extern int rounds_nbr_values[16];

extern pixel_t *(render_buffer[2]);	/* xbuf * ybuf */

char key_or_joy_ready (void);
keycode_t get_key_or_joy (void);

extern char kbjoy[6];
extern char kbjoyold[6];
extern char in_jokebox;
extern char in_menu;
extern char in_demo;

extern char demo_ready;

extern pixel_t glenz[8][256];
void draw_glenz_box (pixel_t *dest, int c, int xt, int yt);

extern pcx_image_t main_font_img, vehicles_img;
extern pcx_image_t bonus_font_img;
extern pcx_image_t tile_set_img, font_deck_img;

extern signed char minisinus[32];
extern bool two_players;

#define maxq 128

extern player_t player[4];
extern int trail_pos[4][maxq];
extern char trail_way[4][maxq];
extern int trail_offset[4];
extern char trail_size[4];	/* size of trails, minus one */

extern int col2plr[4];
extern int plr2col[4];

extern level_t lvl;

extern unsigned char *square_occupied;
extern unsigned char *square_way;
extern tile_index_t *square_tile;
extern square_coord_pair_t *square_coord;
extern signed char *square_object;
#define lemmings_per_players 50
#define lemmings_total (lemmings_per_players*4)
extern lemming_t **square_lemmings_list;
extern lemming_t **square_dead_lemmings_list;
extern lemming_t lemmings_support[lemmings_total];
extern int objects_nbr;

extern int radar_target_pos;
extern int radar_current_pos;

extern int game_mode;
extern gameid_t game_id;

extern int camera_center_x;
extern int lemmings_anim_offset;
extern int lemmings_move_offset;

#endif /* HEROES__CONST__H */
