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

extern int rounds_nbr_values[16];

extern int camera_x[2];		/* fixed point 16b,16b */
extern int camera_y[2];		/* idem */
extern unsigned int corner_x[2];	/* in pixels */
extern unsigned int corner_y[2];	/* idem */
extern pixel_t *(corner[2]);	        /* topleft of the camera vision */

extern unsigned int corner_dx[2];	/* coordinate of the first tile */
extern unsigned int corner_dy[2];	/* idem */
extern int inert_x[2], inert_y[2];
extern unsigned int nbr_tiles_cols;	/* # of tiles columns to display */
extern unsigned int nbr_tiles_rows;	/*        ... rows ... */
extern char camera_stop_x[2];
extern char camera_stop_y[2];

extern pixel_t *(render_buffer[2]);	/* xbuf * ybuf */

char key_or_joy_ready (void);
keycode_t get_key_or_joy (void);

extern char kbjoy[6];
extern char kbjoyold[6];
extern char in_jokebox;
extern char in_menu;
extern char in_demo;

extern char demo_ready;

extern int square_offset[4];

extern int d2w[9];
extern int w2d[4];

extern pixel_t glenz[8][256];
void draw_glenz_box (pixel_t *dest, int c, int xt, int yt);

extern level_header_t map_info;
extern unsigned long int map_info_2xt, map_info_2yt;
extern signed long int map_info_2xwrap, map_info_2ywrap;

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

extern tile_t *level_map;

extern int last_explo;

extern unsigned char *square_occupied;
extern unsigned char *square_way;
extern unsigned char *square_radar_wall;
extern unsigned char *square_wall;
extern unsigned char *square_explosion;
extern int *square_dead_explosion;
extern unsigned char **explo_list_ptr;
extern int *explo_list_pos_x;
extern int *explo_list_pos_y;
extern int explo_nbr;
extern unsigned char *square_explosion_type;
extern int *square2tile;
extern int *square_wrap;
extern int *square_offset2coord;
extern signed char *square_object;
#define lemmings_per_players 50
#define lemmings_total (lemmings_per_players*4)
extern lemming_t **square_lemmings_list;
extern lemming_t **square_dead_lemmings_list;
extern lemming_t lemmings_support[lemmings_total];
extern int objects_nbr;
extern int square2offset[4];

extern int radar_target_pos;
extern int radar_current_pos;

extern int game_mode;
/* extern char questmode; */
extern unsigned char game_magic;

extern int camera_center_x;
extern int lemmings_anim_offset;
extern int lemmings_move_offset;

extern char invincible[4];

#endif /* HEROES__CONST__H */
