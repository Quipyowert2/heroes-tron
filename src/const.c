/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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

/* Heroes constants */

#include "common.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "joystick.h"
#include "pcx.h"
#include "sfx.h"
#include "structs.h"
#include "options.h"
#include "display.h"
#include "const.h"
#include "timer.h"
#include "heroes.h"

/* Def. des 15 sprites de l'explosion1, à l'envers (14->0). */
#define nfrexplo1 15
int fst_explo_list[nfrexplo1] = { 132 + 65 * 320,
  99 + 65 * 320,
  66 + 65 * 320,
  33 + 65 * 320,
  65 * 320,
  264 + 32 * 320,
  231 + 32 * 320,
  198 + 32 * 320,
  165 + 32 * 320,
  132 + 32 * 320,
  99 + 32 * 320,
  66 + 32 * 320,
  33 + 32 * 320,
  32 * 320
};
int snd_explo_list[nfrexplo1] = { 132 + 131 * 320,
  99 + 131 * 320,
  66 + 131 * 320,
  33 + 131 * 320,
  131 * 320,
  264 + 98 * 320,
  231 + 98 * 320,
  198 + 98 * 320,
  165 + 98 * 320,
  132 + 98 * 320,
  99 + 98 * 320,
  66 + 98 * 320,
  33 + 98 * 320,
  98 * 320
};

int trail[16] =
  { 110 * 192, 0, 0, 30 * 192, 100 * 192, 20 * 192, 70 * 192, 0, 0, 10 * 192,
  80 * 192, 40 * 192, 90 * 192, 0, 60 * 192, 50 * 192
};

/* correspondance entree-output pour les virages de tunnels, je sais plus
   comment ça marche, mais ça marche... */
int tunnel_square_io[4][2] = { {0, 1}, {1, 3}, {3, 2}, {2, 0} };

#define xbuf 384		/* pour des multiplications plus faciles */
#define ybuf 300		/* une bande vide de 50 lignes au dessus et en dessous... */
#define sbuf 50*xbuf		/* ...pour éviter de faire du clipping sur les explosions */

#define NOGLENZPLR 108		/* colors pour les trainées sans glenz */
#define NOGLENZRED 16		/* couleur pour le sang sans glenz */

pixel_t radar_trail_color[16] =
  { 111, 127, 143, 159, 111, 127, 143, 159, 109, 125, 141, 157, 109, 125, 141,
  157
};
pixel_t radar_wall_color[16] =
  { 0, 89, 89, 91, 89, 91, 91, 93, 89, 91, 91, 93, 91, 93, 93, 95 };

/* L+  L-  S+  S-  R#   C  ZZ  !!  -1  T+  T-  EL  []   X  XL  ~~  $$ */
int bonus_proba_array[5][17] =
  { {40, 10, 12, 8, 8, 40, 0, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0},	/* quest */
{0, 0, 10, 7, 7, 20, 0, 8, 10, 10, 11, 0, 6, 7, 2, 7, 0},	/* deathm */
{25, 10, 12, 8, 8, 40, 0, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0},	/* killem */
{25, 10, 12, 8, 8, 20, 0, 8, 10, 16, 16, 0, 8, 7, 4, 7, 25},	/* tcash */
{25, 10, 12, 8, 8, 30, 0, 8, 10, 16, 16, 0, 8, 7, 4, 7, 0}
};				/* color */
int bonus_points[2][17] =
  { {20, -15, 15, -10, 5, 18, 0, -10, 0, 0, -5, 50, 5, 0, 8, 0, 25}, /* violets */
{-15, 10, 0, 10, 5, -5, 0, 8, 8, -5, 5, -20, -5, 9, -10, 9, -25}
}; /* beiges */

/* differentes possibilités pour le nbr de rounds */
int rounds_nbr_values[16] =
  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30, 50, 100 };

int square_offset_320[4] = { 0, 12, 320 * 10, 320 * 10 + 12 };
int square_offset[4] = { 0, 12, xbuf * 10, xbuf * 10 + 12 };

char mode_name[5][12] = { "QUEST", "DEATH MATCH", "KILL'EM ALL",
  "TIME CA$H", "COLORS"
};

/* traductions way<->directions */
int d2w[9] = { 0, 0, 1, 1, 2, 2, 2, 2, 3 };
int w2d[4] = { d_up, d_right, d_down, d_left };

char in_menu = 1;
char in_demo = 0;
/* int error; */
char in_jokebox;

char kbjoy[6] = { 0, 0, 0, 0, 0, 0 };
char kbjoyold[6] = { 0, 0, 0, 0, 0, 0 };
palette_t temppal;

unsigned long int camera_x[2];
unsigned long int camera_y[2];
unsigned long int corner_x[2];
unsigned long int corner_y[2];
pixel_t *(corner[2]);

unsigned long int corner_dx[2];
unsigned long int corner_dy[2];
unsigned long int inert_x[2], inert_y[2];
unsigned int nbr_tiles_cols = 15;
unsigned int nbr_tiles_rows = 11;
char camera_stop_x[2];
char camera_stop_y[2];

pixel_t *(render_buffer[2]);		/* 384*260 */

pixel_t glenz[8][256];		/* glenz lines */

level_header_t map_info = { 0, 0, -1, -1, 
			    {0, 0, 0, 0}, {0, 0, 0, 0}, "", "", "" };
unsigned long int map_info_2xt, map_info_2yt;
signed long int map_info_2xwrap, map_info_2ywrap;
palette_t pal;

pcx_image_t main_font_img, icons_img, vehicles_img, trailimg;
pcx_image_t bonus_a_img, bonus_b_img, bonus_font_img, jukebox_img;
pcx_image_t tile_set_img, font_deck_img;

signed char minisinus[32];
char two_players = 0;

/****** JOUEURS ET TRAINEE ******/
#define maxq 128
/* maxq à reporter dans const.h !!! */
player_t player[4];
int trail_pos[4][maxq];
char trail_way[4][maxq];
int trail_offset[4];
char trail_size[4];		/* Taille de la trainée MOINS UN */
/*******************************/

/* correspondance couleur<->player_t */
int col2plr[4];
int plr2col[4];

/****** STOCKAGE DU LEVEL ******/

tile_t *level_map;		/* pointeur sur le level_map du niveau */

int last_explo;

unsigned char *square_occupied;
unsigned char *square_way;
unsigned char *square_radar_wall;
unsigned char *square_wall;
unsigned char *square_explosion;
int *square_dead_explosion;
unsigned char **explo_list_ptr;
int *explo_list_pos_x;
int *explo_list_pos_y;
int explo_nbr;
unsigned char *square_explosion_type;
unsigned char *tile_bonus;
unsigned char *tile_bonus_cpu;
int *square2tile;
int *bonus_time;
unsigned char **bonus_ptr;
int *square_wrap;
int *square_offset2coord;
signed char *square_object;
#define lemmings_per_players 50
#define lemmings_total (lemmings_per_players*4)
/* constantes à reporter dans const.c */
lemming_t **square_lemmings_list;
lemming_t **square_dead_lemmings_list;
lemming_t lemmings_support[lemmings_total];
int bonus_total_nbr, bonus_real_nbr, objects_nbr;
int next_bonus_to_update;
int square2offset[4] = { 0, 1, 0, 0 };	/* deux dernières valeurs calculées plus tard */

int bonus_anim_offset;
int radar_target_pos;
int radar_current_pos;

int game_mode = 0;
/* char questmode=0; */
unsigned char game_magic;
int camera_center_x = 873813;
pixel_t* clock_anim_offset;
int lemmings_anim_offset;
int lemmings_move_offset;

char invincible[4];

char demo_ready = 0;

char
key_or_joy_ready (void)
{
  int i;
  int ct = read_htimer (demo_trigger_htimer);

  if ((in_jokebox == 0)) {
    if (ct >= 30)
      demo_ready = 1;
    else
      demo_ready = 0;
    if (ct >= 60) {
      if (in_menu)
	event_sfx (118 + (rand () & 1));
      reset_htimer (demo_trigger_htimer);
    }
  }

  if (key_ready ()) {
    reset_htimer (demo_trigger_htimer);
    return (1);
  }
  if ((joystick_detected & 1) && (opt.ctrl_one == 1 || opt.ctrl_two == 1)) {
    for (i = 0; i < 6; i++)
      kbjoyold[i] = kbjoy[i];
    get_joystick_state ();
    kbjoy[0] = is_joystick_up (0);
    kbjoy[1] = is_joystick_right (0);
    kbjoy[2] = is_joystick_down (0);
    kbjoy[3] = is_joystick_left (0);
    kbjoy[4] = is_joystick_button_a (0);
    kbjoy[5] = is_joystick_button_b (0);
    if ((kbjoy[0] && !kbjoyold[0])
	|| (kbjoy[1] && !kbjoyold[1])
	|| (kbjoy[2] && !kbjoyold[2])
	|| (kbjoy[3] && !kbjoyold[3])
	|| (kbjoy[4] && !kbjoyold[4])
	|| (kbjoy[5] && !kbjoyold[5])) {
      reset_htimer (demo_trigger_htimer);
      return (1);
    }
  }
  return (0);
}

keycode_t
get_key_or_joy (void)
{
  if (key_ready ())
    return (get_key ());
  if (kbjoy[0] && !kbjoyold[0])
    return (HK_Up);
  if (kbjoy[1] && !kbjoyold[1])
    return (HK_Right);
  if (kbjoy[2] && !kbjoyold[2])
    return (HK_Down);
  if (kbjoy[3] && !kbjoyold[3])
    return (HK_Left);
  if (kbjoy[4] && !kbjoyold[4])
    return (HK_Enter);
  if (kbjoy[5] && !kbjoyold[5])
    return (HK_Escape);
  printf ("get_key_or_joy(): no event.");
  return (0);
}

void
draw_glenz_box (pixel_t *dest, int c, int xt, int yt)
{
  pixel_t *glenzligne = glenz[c];
  int xt2;

  for (; yt != 0; yt--) {
    for (xt2 = xt; xt2 != 0; xt2--)
      *dest++ = glenzligne[*dest];
    dest += xbuf - xt;
  }

}
