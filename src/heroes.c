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

#include "system.h"
#include "display.h"
#include "pcx.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "joystick.h"
#include "errors.h"
#include "fastmem.h"
#include "sfx.h"
#include "options.h"
#include "scores.h"
#include "savegame.h"
#include "intro.h"
#include "menus.h"
#include "draw.h"
#include "extras.h"
#include "visuals.h"
#include "render.h"
#include "renderdata.h"
#include "pixelize.h"
#include "txts.h"
#include "misc.h"
#include "argv.h"
#include "debugmsg.h"

#include "sound.h"
#include "endscroll.h"

#include "heroes.h"
#include "userdir.h"
#include "userconf.h"
#include "musicfiles.h"
#include "endian.h"
#include "hendian.h"
#include "rsc_files.h"
#include "fader.h"

#define __HEROES__

#include "structs.h"
#include "const.h"
#include "scrtools.h"
#include "fontdata.h"
#include "bonus.h"

char tile_set_name[128];
char glenz_name[128];

char *level_list;
#define levellstchunk 13
char *levelinf;
int *level_full_list;
int level_full_list_size = 0;
int rounds = 1;

int level_list_nbr = 0;

int current_quest_level;

char soundtrack_list[11][9] = { "MENU", "HEROES01", "HEROES02", "HEROES03",
  "HEROES04", "HEROES05", "HEROES06", "HEROES07",
  "HEROES08", "HEROES09", "HEROES10"
};

signed char soundtrack_current_nbr = 0;

int level_is_finished;

/* AI won't be enabled on the first moves */
#define ia_skip_firsts_moves 2

char ia_max_depth;
char ia_cur_depth;
char ia_is_invincible;
char ia_player;
int ia_target_x;
int ia_targer_y;
int ia_wrap_x, ia_wrap_y;
char ia_wrap_left, ia_wrap_right;

#define DEMO_DURATION 90

char enable_blit;

char mouse_found = 1;
/***********************/

char txt_tmp[20];

htimer_t blink_htimer;
htimer_t clock_htimer;
htimer_t tiles_anim_htimer;
htimer_t corner_htimer;
htimer_t event_htimer;
long event_time;		/* updated from event_htimer on each frame */
htimer_t update_htimer;
htimer_t waving_htimer;
htimer_t background_htimer;
htimer_t sound_track_htimer;
htimer_t demo_trigger_htimer;

#include "gfx_reader.h"

static unsigned char play_game (char);

static void
init_buffers (void)
{
  render_buffer[0] = malloc (xbuf * ybuf);
  render_buffer[1] = malloc (xbuf * ybuf);
  if ((render_buffer[0]) == NULL || (render_buffer[1]) == NULL)
    emsg ("init_buffer(): mem error");
}

static void
clean_buffers (void)
{
  memset (render_buffer[0], 0, xbuf * ybuf);
  memset (render_buffer[1], 0, xbuf * ybuf);
}

static void
close_buffers (void)
{
  free (render_buffer[0]);
  free (render_buffer[1]);
}

static void
add_color (char oui)
{
  int d;
  unsigned char b;
  do {
    d = rand () % (map_info_2xt * map_info_2yt);
  }
  while (square_object[d] != -1);
  if (oui && (rand () % 40 == 0))
    b = 16;
  else
    b = rand () % 5;
  if ((rand () & 3) == 0)
    b |= 8;
  square_object[d] = b;
}

static void
add_cash (char oui)
{
  int d;
  unsigned char b = 0;
  do {
    d = rand () % (map_info_2xt * map_info_2yt);
  }
  while (square_object[d] != -1);
  if (oui && (rand () % 40 == 0))
    b = 15;
  square_object[d] = b;
}

static void
erase_player (int i)
{
  dmsg (D_MISC, "erase player %d", i);

  if (!in_menu)
    square_occupied[player[i].x2 + player[i].y2 * map_info_2xt] = 0xff;
  if (player[i].way == w_left)
    square_occupied[player[i].y2 * map_info_2xt +
		    ((player[i].x2 - 1) & (map_info_2xwrap))] = 0xff;
  if (player[i].way == w_right)
    square_occupied[player[i].y2 * map_info_2xt +
		    ((player[i].x2 + 1) & (map_info_2xwrap))] = 0xff;
  if (player[i].way == w_up)
    square_occupied[player[i].x2 +
		    ((player[i].y2 - 1) & (map_info_2ywrap)) * map_info_2xt] =
      0xff;
  if (player[i].way == w_down)
    square_occupied[player[i].x2 +
		    ((player[i].y2 + 1) & (map_info_2ywrap)) * map_info_2xt] =
      0xff;
}


static void
reinit_player (int i)
{
  int j, m, d;
  char unusable;
  unsigned char k, l;

  dmsg (D_MISC, "initialize player %d", i);

  j = 4;
  k = (unsigned char) (i - 1);
  do {
    j--;
    k = (unsigned char) ((k + 1) & 3);
  } while (j != 0 && square_occupied[
				     (map_info.start[k] % map_info.xt) * 2 +
				     ((map_info.start_way[k] & 15) == 1
				      || (map_info.start_way[k] & 15) ==
				      3) +
				     ((map_info.start[k] / map_info.xt) * 2 +
				      ((map_info.start_way[k] & 15) == 2
				       || (map_info.start_way[k] & 15) ==
				       3)) * map_info_2xt] != 0xff);

  player[i].way = (char) (map_info.start_way[k] >> 4);
  player[i].square = (char) (map_info.start_way[k] & 15);
  player[i].x = map_info.start[k] % map_info.xt;
  player[i].x2 = player[i].x * 2 + (player[i].square == 1
				    || player[i].square == 3);
  player[i].y = map_info.start[k] / map_info.xt;
  player[i].y2 = player[i].y * 2 + (player[i].square == 2
				    || player[i].square == 3);

  /* ensure that the start position is usable,
     otherwise try another position (randomly) */

  do {
    char bitfl[4];
    unusable = 0;
    bitfl[3] = bitfl[2] = bitfl[1] = bitfl[0] = 0;
    d = player[i].x2 + player[i].y2 * map_info_2xt;

    /* is the square closed or already occupied? */
    if (square_wall[d] == 15 || square_occupied[d] != 0xff)
      unusable = 1;
    else {
      /* for each direction, there must not be a wall, neither shall
         there be an occupied square */
      if (square_wrap[(d << 2) + 0] == -1)
	bitfl[0] = 1;
      else if (square_occupied[square_wrap[(d << 2) + 0]] != 0xff)
	bitfl[0] = 1;
      if (square_wrap[(d << 2) + 1] == -1)
	bitfl[1] = 1;
      else if (square_occupied[square_wrap[(d << 2) + 1]] != 0xff)
	bitfl[1] = 1;
      if (square_wrap[(d << 2) + 2] == -1)
	bitfl[2] = 1;
      else if (square_occupied[square_wrap[(d << 2) + 2]] != 0xff)
	bitfl[2] = 1;
      if (square_wrap[(d << 2) + 3] == -1)
	bitfl[3] = 1;
      else if (square_occupied[square_wrap[(d << 2) + 3]] != 0xff)
	bitfl[3] = 1;
      /* is at least one direction ok? */
      if ((bitfl[0] + bitfl[1] + bitfl[2] + bitfl[3]) == 4)
	unusable = 1;
      else
	/* rotate until we find a direction */
	while (bitfl[player[i].way] == 1)
	  player[i].way = (player[i].way + 1) & 3;
    };
    if (unusable) {
      /* get a new position randomly on the map */
      player[i].square = rand () & 3;
      player[i].x = rand () % map_info.xt;
      player[i].x2 = player[i].x * 2 + (player[i].square == 1
					|| player[i].square == 3);
      player[i].y = rand () % map_info.yt;
      player[i].y2 = player[i].y * 2 + (player[i].square == 2
					|| player[i].square == 3);
    }
  } while (unusable);
/**************/

  player[i].ia_max_depth = (rand () & 1) + 5;
  if (game_mode >= M_KILLEM && game_mode != M_DEATHM)
    player[i].behaviour = 2;
  else
    player[i].behaviour = rand () & 1;

  if (two_players)
    player[i].target = col2plr[rand () & 1];
  else
    player[i].target = col2plr[0];

  player[i].target |= (ia_skip_firsts_moves * 16);

  player[i].d.e = 0;
  player[i].turbo = 1;
  player[i].turbo_level_delta = player[i].turbo_level = 1024;
  player[i].vitp = player[i].v = 4369 * 3 / 2 + (opt.speed * 2 * 1092);
  player[i].spec = 0;
  player[i].delay = 0;
  player[i].inversed_controls = 0;
  player[i].speedup = 0;
  player[i].rotozoom = 0;
  player[i].waves = 0;
  player[i].waves_begin = 0;
  player[i].tunnel_inverse = 0;
  player[i].next_way = player[i].old_old_way = player[i].old_way =
    player[i].way;
  if (!in_menu)
    square_occupied[player[i].x2 + player[i].y2 * map_info_2xt] = i;
  if (player[i].way == w_left)
    square_occupied[player[i].y2 * map_info_2xt +
		    ((player[i].x2 - 1) & (map_info_2xwrap))] =
      (char) (i + 4);
  if (player[i].way == w_right)
    square_occupied[player[i].y2 * map_info_2xt +
		    ((player[i].x2 + 1) & (map_info_2xwrap))] =
      (char) (i + 4);
  if (player[i].way == w_up)
    square_occupied[player[i].x2 +
		    ((player[i].y2 - 1) & (map_info_2ywrap)) * map_info_2xt] =
      (char) (i + 4);
  if (player[i].way == w_down)
    square_occupied[player[i].x2 +
		    ((player[i].y2 + 1) & (map_info_2ywrap)) * map_info_2xt] =
      (char) (i + 4);
/*     explofr[i]=0; */
  j = player[i].x2 + player[i].y2 * map_info_2xt;
  l = player[i].way;
  square_way[j] = (char) ((player[i].way << 2) + player[i].way);
  trail_offset[i] = 0;
  for (m = /* trail_offset[i]+ */ trail_size[i] /* -1 */ ;
       m >= 0 /* trail_offset[i] */ ;
       m--) {
    trail_pos[i][m] = j;
    trail_way[i][m] = (char) ((l << 2) + l);
  }

}

static char
load_level (char *nomlvl, char cont)
{
  FILE *ftmp;
  int i, j, k, k2, l, m;
  int *ptr;
  lemming_t *ptir;

  dmsg (D_FILE|D_LEVEL, "loading level: %s", nomlvl);

  if (!in_menu) {
    if (two_players) {
      player[col2plr[0]].cpu = 2;
      player[col2plr[1]].cpu = 2;
      player[col2plr[2]].cpu = 0;
      player[col2plr[3]].cpu = 0;
    } else {
      player[col2plr[0]].cpu = 2;
      player[col2plr[1]].cpu = 0;
      player[col2plr[2]].cpu = 0;
      player[col2plr[3]].cpu = 0;
    }
  }
  if (in_demo) {
    player[col2plr[0]].cpu = 0;
    player[col2plr[1]].cpu = 0;
  }

  clean_buffers ();

  if ((ftmp = fopen (nomlvl, "rb")) == NULL) {
    dmsg (D_LEVEL|D_FILE, "cannot open %s", nomlvl);
    dperror ("fopen");
    return (1);
  }
  {
    char* tmp = get_non_null_rsc_file ("tiles-sets-dir");
    strcpy (tile_set_name, tmp);
    strcpy (glenz_name, tmp);
    free (tmp);
  }
  dmsg (D_FILE|D_LEVEL, "read level header");
  if (fread (&map_info, sizeof (level_header_t), 1, ftmp) != 1)
    return (2);

  /* convert map_info to local endianess */
  bswap_level_header (&map_info);

  dmsg (D_LEVEL, "size=(%u,%u) wrap=(%x,%x) tile=%s soundtrack=%s",
	map_info.xt, map_info.yt,
	map_info.xwrap, map_info.ywrap,
	map_info.tile_set_name, map_info.soundtrack_name);

  level_map = malloc (map_info.xt * map_info.yt * sizeof (tile_t));
  if (level_map == NULL)
    return (3);

  dmsg (D_LEVEL, "reading full level map (%d bytes)",
	map_info.xt * map_info.yt * sizeof(tile_t));

  i = fread (level_map, sizeof (tile_t), map_info.xt * map_info.yt, ftmp);
  if (i != (int)(map_info.xt * map_info.yt))
    return (4);

  /* convert level_map to local endianess */
  bswap_level_tiles (&map_info, level_map);

  fclose (ftmp);
  strlwr (map_info.tile_set_name);
  strcat (strcat (tile_set_name, map_info.tile_set_name), ".pcx");
  strcat (strcat (glenz_name, map_info.tile_set_name), ".glz");
  pcx_load (tile_set_name, &tile_set_img);
  dmsg (D_LEVEL|D_FILE, "loading %s", glenz_name);
  if ((ftmp = fopen (glenz_name, "rb")) == NULL) {
    dperror ("fopen");
    return (7);
  }
  if (fread (glenz, 256, 8, ftmp) != 8)
    return (6);
  fclose (ftmp);

  init_render_data ();

  if (in_menu) {
    load_soundtrack_from_alias ("MENU");
    soundtrack_current_nbr = 0;
  } else {
    load_soundtrack_from_alias (map_info.soundtrack_name);
    if (map_info.soundtrack_name[7] == '0')
      soundtrack_current_nbr = 10;
    else
      soundtrack_current_nbr = map_info.soundtrack_name[7] - '0';
  }

  dmsg (D_LEVEL, "initialize variables and maps associated to the level");

  map_info_2xt = map_info.xt << 1;
  map_info_2yt = map_info.yt << 1;
  map_info_2xwrap = (map_info.xwrap << 1) + 1;
  map_info_2ywrap = (map_info.ywrap << 1) + 1;
  square_occupied = malloc (map_info_2xt * map_info_2yt *
			    sizeof (*square_occupied));
  if (square_occupied == NULL)
    return (9);
  memset (square_occupied, 0xff, map_info_2xt * map_info_2yt);
  square_radar_wall = malloc (map_info_2xt * map_info_2yt *
			      sizeof (*square_radar_wall));
  if (square_radar_wall == NULL)
    return (10);
  memset (square_radar_wall, 0, map_info_2xt * map_info_2yt);
  square_wall = malloc (map_info_2xt * map_info_2yt *
                        sizeof (*square_wall));
  if (square_wall == NULL)
    return (11);
  memset (square_wall, 0, map_info_2xt * map_info_2yt);
  square_explosion = malloc ((map_info_2xt * map_info_2yt + 1) *
		     sizeof (*square_explosion));
  /* +1 ?????? */
  if (square_explosion == NULL)
    return (12);
  memset (square_explosion, 254,
	  (map_info_2xt * map_info_2yt + 1) * sizeof (*square_explosion));
  square_dead_explosion =
    malloc ((map_info_2xt * map_info_2yt + 1) * sizeof (int));
    /* ??? */
  if (square_dead_explosion == NULL)
    return (12);
  memset (square_dead_explosion, 0,
	  (map_info_2xt * map_info_2yt + 1) * sizeof (int));
  last_explo = 0;
  square_explosion_type = malloc ((map_info_2xt * map_info_2yt + 1) *
			          sizeof (*square_explosion_type));
  if (square_explosion_type == NULL)
    return (13);

  for (i = map_info_2xt * map_info_2yt - 1; i >= 0; i--)
    square_explosion_type[i] = rand () & 1;
  square_way = malloc (map_info_2xt * map_info_2yt * sizeof (*square_way));
  if (square_way == NULL)
    return (14);

  if (init_bonuses_level ())
    return 15;

  square2tile = malloc (map_info_2xt * map_info_2yt * sizeof (*square2tile));
  if (square2tile == NULL)
    return (15);

  k = 0;

  for (i = 0, l = 0; i < (int)map_info.yt; i++, l += map_info_2xt * 2) {
    for (j = 0, k2 = l; j < (int)map_info.xt; j++, k2 += 2, k++) {
      square2tile[k2] = k;
      square2tile[k2 + 1] = k;
      square2tile[map_info_2xt + k2] = k;
      square2tile[map_info_2xt + k2 + 1] = k;
    }
  }

  square_wrap =
    malloc (map_info_2xt * map_info_2yt * 4 * sizeof (*square_wrap));
  if (square_wrap == NULL)
    return (16);
  memset (square_wrap, 0,
	  map_info_2xt * map_info_2yt * 4 * sizeof (*square_wrap));
  square_offset2coord = malloc (map_info_2xt * map_info_2yt * 2 *
                                sizeof (*square_offset2coord));
  if (square_offset2coord == NULL)
    return (17);
  memset (square_offset2coord, 0,
	  map_info_2xt * map_info_2yt * 2 * sizeof (*square_offset2coord));
  if (game_mode == M_KILLEM) {
    square_lemmings_list = malloc ((map_info_2xt * map_info_2yt) *
                                   sizeof (lemming_t *));
    square_dead_lemmings_list = malloc ((map_info_2xt * map_info_2yt) *
                                        sizeof (lemming_t *));
    /* lemmings_support= malloc(lemmings_total*sizeof(lemming_t)); */
    if (square_lemmings_list == NULL
	|| /*lemmings_support==NULL || */ square_dead_lemmings_list == NULL)
      return (18);
    memset (square_lemmings_list, 0,
	    (map_info_2xt * map_info_2yt) * sizeof (lemming_t *));
    memset (square_dead_lemmings_list, 0,
	    (map_info_2xt * map_info_2yt) * sizeof (lemming_t *));
    memset (lemmings_support, 0, lemmings_total * sizeof (lemming_t));
  }
  if (game_mode >= M_TCASH) {
    square_object = malloc (map_info_2xt * map_info_2yt *
                            sizeof (*square_object));
    if (square_object == NULL)
      return (19);
  }

  square2offset[2] = map_info_2xt;
  square2offset[3] = map_info_2xt + 1;

  j = 0;
  l = 0;
  for (k = 0; k < (int)map_info.yt; k++) {
    for (i = 0; i < (int)map_info.xt; i++) {
      square_radar_wall[j] = level_map[i + l].collision[0];
      square_radar_wall[j + 1] = level_map[i + l].collision[1];
      square_radar_wall[j + map_info_2xt] = level_map[i + l].collision[2];
      square_radar_wall[j + map_info_2xt + 1] = level_map[i + l].collision[3];
      if (level_map[i + l].type == t_boom) {
	if (level_map[i + l].info.param[0] != 0)
	  square_explosion[j] = 255;
	if (level_map[i + l].info.param[1] != 0)
	  square_explosion[j + 1] = 255;
	if (level_map[i + l].info.param[2] != 0)
	  square_explosion[j + map_info_2xt] = 255;
	if (level_map[i + l].info.param[3] != 0)
	  square_explosion[j + map_info_2xt + 1] = 255;
      }
      j += 2;
    }
    j += map_info_2xt;
    l += map_info.xt;
  }
  if (map_info.ywrap != DONT_WRAP) {
    for (i = 0; i < (int)map_info_2xt; i++) {
      if (square_radar_wall[(map_info_2ywrap) * map_info_2xt + i] & c_down)
	square_wall[i] |= d_up;
      if (square_radar_wall[i] & c_up)
	square_wall[(map_info_2ywrap) * map_info_2xt + i] |= d_down;
    }
  } else
    for (i = 0; i < (int)map_info_2xt; i++) {
      square_wall[i] |= d_up;
      square_wall[(map_info_2yt - 1) * map_info_2xt + i] |= d_down;
    };
  if (map_info.xwrap != DONT_WRAP) {
    for (i = 0; i < (int)map_info_2yt; i++) {
      if (square_radar_wall[i * map_info_2xt + map_info_2xwrap] & c_right)
	square_wall[i * map_info_2xt] |= d_left;
      if (square_radar_wall[i * map_info_2xt] & c_left)
	square_wall[map_info_2xwrap + i * map_info_2xt] |= d_right;
    }
  } else
    for (i = 0; i < (int)map_info_2yt; i++) {
      square_wall[i * map_info_2xt] |= d_left;
      square_wall[map_info_2xt - 1 + i * map_info_2xt] |= d_right;
    };
  j = 0;
  for (k = map_info_2yt - 1; k != 0; k--) {
    for (i = 0; i < (int)map_info_2xt; i++) {
      if (square_radar_wall[j + i] & c_down)
	square_wall[j + i + map_info_2xt] |= d_up;
      if (square_radar_wall[j + i + map_info_2xt] & c_up)
	square_wall[j + i] |= d_down;
    }
    j += map_info_2xt;
  }
  j = 0;
  for (k = map_info_2yt; k != 0; k--) {
    for (i = 0; i < (int)map_info_2xt - 1; i++) {
      if (square_radar_wall[j + i] & c_right)
	square_wall[j + i + 1] |= d_left;
      if (square_radar_wall[j + i + 1] & c_left)
	square_wall[j + i] |= d_right;
    }
    j += map_info_2xt;
  }

  explo_nbr = 0;
  for (i = map_info.xt * map_info.yt * 4 - 1; i >= 0; i--)
    if (square_explosion[i] == 255)
      explo_nbr++;
  if (explo_nbr != 0) {
    explo_list_ptr = malloc (explo_nbr * sizeof (*explo_list_ptr));
    if (explo_list_ptr == NULL)
      return (34);
    explo_list_pos_x = malloc (explo_nbr * sizeof (*explo_list_pos_x));
    if (explo_list_pos_x == NULL)
      return (35);
    explo_list_pos_y = malloc (explo_nbr * sizeof (*explo_list_pos_x));
    if (explo_list_pos_y == NULL)
      return (36);
    j = 0;
    for (i = map_info.xt * map_info.yt * 4 - 1; i >= 0; i--)
      if (square_explosion[i] == 255) {
	explo_list_ptr[j] = square_explosion + i;
	explo_list_pos_x[j] = i % (map_info_2xt);
	explo_list_pos_y[j] = i / (map_info_2xt);
	j++;
      }
  }
  /* init square_wrap, the map of moves */
  ptr = (int *) square_wrap;
  k = 0;
  for (j = 0; j < (int)map_info_2yt; j++)
    for (i = 0; i < (int)map_info_2xt; i++) {
      if (square_wall[k] & d_up)
	l = -1;
      else
	l = i + ((j - 1) & (map_info_2ywrap)) * map_info_2xt;
      *ptr++ = l;
      if (square_wall[k] & d_right)
	l = -1;
      else
	l = ((i + 1) & (map_info_2xwrap)) + j * map_info_2xt;
      *ptr++ = l;
      if (square_wall[k] & d_down)
	l = -1;
      else
	l = i + ((j + 1) & (map_info_2ywrap)) * map_info_2xt;
      *ptr++ = l;
      if (square_wall[k] & d_left)
	l = -1;
      else
	l = ((i - 1) & (map_info_2xwrap)) + j * map_info_2xt;
      *ptr++ = l;
      k++;
    }
  l = 0;
  m = 0;
  for (j = 0; j < (int)map_info.yt; j++) {	/* search for tunnels */
    for (i = 0, k = 0; i < (int)map_info.xt; i++, m++, k += 8) {
      if (level_map[m].type == t_tunnel) {
	unsigned int way = d2w[level_map[m].info.tunnel.direction];
	unsigned int output = level_map[m].info.tunnel.output;
	unsigned int dest_way;

	/* find which way should the vehicule seem to come from when
	   it quit the tunnel */

	if (level_map[output].type == t_tunnel)
	  /* if the destination tile is a tunnel, use its direction */
	  dest_way = d2w[level_map[output].info.tunnel.direction];
	else
	  /* else, consider the output as an imaginary tunnel whose
	     direction is reversed */
	  dest_way = way ^ 2;

	/* each tunnel has two input/output squares that must be
	   linked to the two corresponding i/o squares of the
	   destination tile */

	square_wrap[k + l /* current tile */
		   + (square2offset [tunnel_square_io [way][0]] << 2) /* sqr */
		   + way /* way */] =
	  ((output % map_info.xt) << 1)
	  + ((output / map_info.xt) << 1) * map_info_2xt
	  + square2offset[tunnel_square_io[dest_way][1]];

	square_wrap[k + l /* current tile */
		   + (square2offset [tunnel_square_io [way][1]] << 2) /* sqr */
		   + way /* way */] =
	  ((output % map_info.xt) << 1)
	  + ((output / map_info.xt) << 1) * map_info_2xt
	  + square2offset[tunnel_square_io[dest_way][0]];
      }
    }
    l += map_info.xt * 4 * 4;
  }
  /* init square_offset2coord, map associating coordinates to offsets */
  ptr = (int *) square_offset2coord;
  for (j = 0; j < (int)map_info_2yt; j++)
    for (i = 0; i < (int)map_info_2xt; i++) {
      *ptr++ = i;
      *ptr++ = j;
    }

  /* init of players  */
  if (!in_menu)
    for (i = 3; i >= 0; i--) {
      /* trail_offset[i]=0; */
      if (game_mode == M_DEATHM) {
	trail_size[i] = 32;
	player[i].lifes = 9;
      } else
	trail_size[i] = 5;
      reinit_player ((char) i);
      if (cont == 0) {
	player[i].lifes = 9;
	player[i].score = 0;
	player[i].wins = 0;
      } else {
	/* reinitialize dead computers: give them an empty score
	   and decrase their total of wins */
	if (player[i].cpu < 2 && player[i].lifes == 0)
	  {
	    player[i].lifes = 9;
	    player[i].score = 0;
	    if (player[i].wins > 0)
	      --player[i].wins;
	  }
      }
      player[i].autopilot = 1;
      player[i].score_delta = player[i].score << 2;
      player[i].invincible = 0;
      player[i].time = 3000;
      player[i].cash = 0;
      player[i].martians_nbr = 0;
    }
  /* reinit player once again to avoid the case where
     some vehicles could have been put in front of others */
  erase_player (0);
  reinit_player (0);
  erase_player (1);
  reinit_player (1);
  erase_player (2);
  reinit_player (2);
  erase_player (3);
  reinit_player (3);

  if (!opt.autopilot_one)
    player[col2plr[0]].autopilot = 0;
  if (two_players && !opt.autopilot_two)
    player[col2plr[1]].autopilot = 0;

  /*** init of lemmings ***/
  if (game_mode == M_KILLEM) {
    ptir = lemmings_support;
    l = (map_info_2xt * map_info_2yt);
    for (i = 0; i < 4; i++) {
      for (j = lemmings_per_players; j != 0; j--) {
	do {
	  do {
	    k = rand () % l;
	    assert (k < (int)(map_info_2xt * map_info_2yt));
	  } while (square_wall[k] == 15 || square_occupied[k] != 0xff
		   || square_lemmings_list[k] != NULL);
	  m = 0;
	  while (square_wall[k] & (1 << m))
	    m++;
	  k2 = square_wrap[(k << 2) + m];
	} while (square_occupied[k2] != 0xff
		 || square_lemmings_list[k2] != NULL);
	assert (m < 4);
	ptir->pos1 = k;
	ptir->pos2 = k2;
	ptir->min = 0;
	ptir->nexttache = NULL;
	ptir->way = m;
	ptir->couleur = i;
	ptir->dead = 0;
	square_lemmings_list[k] = ptir;
	square_lemmings_list[k2] = ptir;
	ptir++;
      }
      player[i].lemmings_nbr = lemmings_per_players;
    }
    assert (ptir == lemmings_support + 4 * lemmings_per_players);
    lemmings_move_offset = 0;
  }

  if (game_mode >= M_TCASH) {
    for (i = map_info_2xt * map_info_2yt - 1; i >= 0; i--)
      if (square_wall[i] == 15)
	square_object[i] = -2;	/* -2 = you can't drive here */
      else
	square_object[i] = -1;
    if (game_mode == M_COLOR) {
      objects_nbr = map_info_2xt * map_info_2yt / 14 + 1;
      for (i = objects_nbr; i != 0; i--)
	add_color (1);
    }
    if (game_mode == M_TCASH) {
      objects_nbr = map_info_2xt * map_info_2yt / 13 + 1;
      for (i = objects_nbr; i != 0; i--)
	add_cash (1);
    }
  }
/* * * * * * * * * * * * * * * * * * */
  level_is_finished = 0;

  /* reset_htimer_with_offset (0, HZ(70)*1000); * what it is intended for? * */
  /* update_htimer (); */
  if (!in_menu)
    spread_bonuses ();
  return (0);
}

static void
unload_level (void)
{
  dmsg (D_LEVEL, "unloading level");

  uninit_render_data ();
  uninit_bonuses_level ();
  img_free (&tile_set_img);
  free (level_map);
  free (square_occupied);
  free (square_explosion);
  free (square_dead_explosion);
  if (explo_nbr != 0) {
    free (explo_list_pos_x);
    free (explo_list_pos_y);
    free (explo_list_ptr);
  }
  free (square_explosion_type);
  free (square_radar_wall);
  free (square_wall);
  free (square_way);
  free (square2tile);
  free (square_wrap);
  free (square_offset2coord);
  if (game_mode == M_KILLEM && !in_menu) {
#ifdef PORT
    /* what was the use of this ? */
    memset (square_lemmings_list, 0,
	    (map_info_2xt * map_info_2yt) * sizeof (lemming_t *));
    memset (square_dead_lemmings_list, 0,
	    (map_info_2xt * map_info_2yt) * sizeof (lemming_t *));
#endif
    free (square_lemmings_list);
    free (square_dead_lemmings_list);
  }
  if (game_mode >= M_TCASH && !in_menu)
    free (square_object);
  unload_soundtrack ();
}

extern void
compute_corner (int p, int n)
{
  int x, y;
  int d1, d2, d3;

  if (opt.inertia) {
    if (map_info.xwrap == DONT_WRAP)
      inert_x[p] = camera_x[p] =
	inert_x[p] + n * ((int)camera_x[p] - (int)inert_x[p]) / 16;
    else {
      d1 = ((int)camera_x[p] - (int)inert_x[p]);
      d3 = abs (d1);
      d2 = (map_info.xt << 16) - d3;
      if (d3 <= d2)
	inert_x[p] = camera_x[p] = inert_x[p] + n * (d1) / 16;
      else if (d1 <= 0)
	inert_x[p] = camera_x[p] =
	  inert_x[p] + n * d2 / 16 - (map_info.xt << 16);
      else
	inert_x[p] = camera_x[p] =
	  inert_x[p] - n * d2 / 16 + (map_info.xt << 16);
    }
    if (map_info.ywrap == DONT_WRAP)
      inert_y[p] = camera_y[p] =
	inert_y[p] + n * ((int)camera_y[p] - (int)inert_y[p]) / 16;
    else {
      d1 = ((int)camera_y[p] - (int)inert_y[p]);
      d3 = abs (d1);
      d2 = (map_info.yt << 16) - d3;
      if (d3 <= d2)
	inert_y[p] = camera_y[p] = inert_y[p] + n * (d1) / 16;
      else if (d1 <= 0)
	inert_y[p] = camera_y[p] =
	  inert_y[p] + n * d2 / 16 - (map_info.yt << 16);
      else
	inert_y[p] = camera_y[p] =
	  inert_y[p] - n * d2 / 16 + (map_info.yt << 16);
    }
  }

  camera_x[p] += 81920 * 2 / 3;
  camera_y[p] += 49152;
  camera_stop_y[p] = camera_stop_x[p] = 0;
  x = (camera_x[p] - (nbr_tiles_cols << 15));
  y = (camera_y[p] - (nbr_tiles_rows << 15));
  if (map_info.xwrap == DONT_WRAP) {
    if (x < 0) {
      x = 0;
      camera_stop_x[p] = 1;
    } else if (x > (int)(map_info.xt << 16) - camera_center_x) {
      x = (map_info.xt << 16) - camera_center_x;
      camera_stop_x[p] = 1;
    }
  }
  if (map_info.ywrap == DONT_WRAP) {
    if (y < 0) {
      y = 0;
      camera_stop_y[p] = 1;
    } else if (y > (int)(map_info.yt << 16) - 655360) {
      y = (map_info.yt << 16) - 655360;
      camera_stop_y[p] = 1;
    }
  }
  corner_dx[p] = (x >> 16) & map_info.xwrap;
  corner_dy[p] = (y >> 16) & map_info.ywrap;
  corner_x[p] = ((x & 0xffff) * 24) >> 16;
  corner_y[p] = ((y & 0xffff) * 20) >> 16;
  corner[p] = render_buffer[p] + sbuf + corner_y[p] * xbuf + corner_x[p];
  /* corner_dy[p]=corner_dx[p]=0; */
  /* printf("Camera \%d\t\%d\nCorner \%d\t\%d\n",camera_x,camera_y,corner_x,corner_y); */
}

static void
write_rle (unsigned char *src, int t, FILE * fpcx)
{
  int oldc, newc;
  int i;
  int nbr = 1;

  oldc = *src++;

  for (i = 1; i < t; i++) {
    newc = *src++;
    if (nbr == 63 || (nbr > 1 && (newc != oldc))) {
      putc (nbr | 192, fpcx);
      putc (oldc, fpcx);
      oldc = newc;
      nbr = 1;
    } else if (oldc == newc && nbr < 63)
      nbr++;
    else {
      if (oldc < 192)
	putc (oldc, fpcx);
      else {
	putc (193, fpcx);
	putc (oldc, fpcx);
      }
      oldc = newc;
    }
  }
  if (nbr == 1) {
    if (oldc < 192)
      putc (oldc, fpcx);
    else {
      putc (193, fpcx);
      putc (oldc, fpcx);
    }
  } else {
    putc (nbr | 192, fpcx);
    putc (oldc, fpcx);
  }
}

static void
save_pcx (char q)
{
  FILE *fpcx;
  static char nompcx[13];
  static int pcxnbr;
  pcx_header_t headpcx;
  int i1;

  headpcx.signature = 10;
  headpcx.version = 5;
  headpcx.rle = 1;
  headpcx.bits_per_pixels = 8;
  headpcx.x = headpcx.y = BSWAP16 (0);
  headpcx.widthdpi = BSWAP16 (0);
  headpcx.heightdpi = BSWAP16 (0);
  headpcx.palette_kind = BSWAP16 (1);
  headpcx.nbrplanes = 1;
  if (q == 0) {
    headpcx.width = BSWAP16 (320 - 1);
    headpcx.height = BSWAP16 (200 - 1);
    headpcx.bytes_per_lines = BSWAP16 (320);
  } else {
    headpcx.width = BSWAP16 (xbuf - 1);
    headpcx.height = BSWAP16 (2 * ybuf);
    headpcx.bytes_per_lines = BSWAP16 (xbuf);
  }

  sprintf (nompcx, "snap%.4d.pcx", pcxnbr++);

  dmsg (D_MISC|D_FILE, "save pcx: %s", nompcx);

  if ((fpcx = fopen (nompcx, "wb")) == NULL) {
    dperror ("fopen");
    return;
  }
  fwrite ((char *) &headpcx, 1, sizeof (pcx_header_t), fpcx);

  if (q == 0)
    write_rle (screen, 320 * 200, fpcx);
  else {
    write_rle ((unsigned char *) render_buffer[0], xbuf * ybuf, fpcx);
    for (i1 = xbuf; i1; i1--)
      putc (15, fpcx);
    write_rle ((unsigned char *) render_buffer[1], xbuf * ybuf, fpcx);
  }
  putc (0xC, fpcx);
  for (i1 = 0; i1 < 768; i1++)
    putc (tile_set_img.palette.global[i1] << 2, fpcx);
  fclose (fpcx);
}

static void
jukebox_menu (void)
{
  int t, t2, dp;
  keycode_t k;
  signed char sinl;
  char l = 0;
  htimer_t lemming_htimer = new_htimer (T_GLOBAL, HZ (18));

  in_jokebox = 1;
  std_white_fadein (&tile_set_img.palette);
  do {
    do {
      background_menu ();

      sinl = minisinus[read_htimer (waving_htimer) & 31];
      draw_glenz_box (corner[0] + (42 + sinl) * xbuf + 234, 2, 86, 6);
      draw_glenz_box (corner[0] + (62 + sinl) * xbuf + 244, 3, 76, 6);
      draw_glenz_box (corner[0] + (74 + sinl) * xbuf + 194, 4, 126, 6);
      draw_glenz_box (corner[0] + (95 + sinl) * xbuf + 194, 5, 126, 6);
      draw_text_waving ("CREDITS", 159, 10, 1);
      draw_text ("GFX AND IDEA:", 1, 40, 0);
      draw_text_waving ("a GUEN", 318, 40, 2);
      draw_text ("MUSIK:", 1, 60, 0);
      draw_text_waving ("b TNK", 318, 60, 2);
      draw_text_waving ("c ALEXEL", 318, 72, 2);
      draw_text ("CODE:", 1, 93, 0);
      draw_text_waving ("d POLLUX", 318, 93, 2);
      draw_text ("SEE THE FILE", 159, 118, 1);
      draw_text ("THANKS", 159, 130, 1);
      draw_text ("FOR OTHER", 159, 142, 1);
      draw_text ("CONTRIBUTORS", 159, 154, 1);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (28) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (109) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (171) * xbuf + 100, 120, 3);

      dp = read_htimer (sound_track_htimer);
      t = dp/2;
      dp &= 1;
      if (t > 5999)
	t = 5999;
      copy_rect_transp (jukebox_img.buffer, corner[0] + 180 * xbuf + 8, 306,
			19);
      if (l == 0)
	copy_rect_4 (jukebox_img.buffer + 19 * 320,
		     corner[0] + 184 * xbuf + 8 + 5, 12, 9);
      else if (l == 1)
	copy_rect_4 (jukebox_img.buffer + 19 * 320 + 12,
		     corner[0] + 184 * xbuf + 8 + 27, 12, 9);
      else if (l == 2)
	copy_rect_4 (jukebox_img.buffer + 19 * 320 + 24,
		     corner[0] + 184 * xbuf + 8 + 274, 16, 9);

      if (soundtrack_title)
	draw_deck_text (soundtrack_title, 110, 186, 1);
      if (soundtrack_author)
	draw_deck_text (soundtrack_author, 197, 186, 1);

      t2 = t % 60;
      t /= 60;
      copy_rect_2 (jukebox_img.buffer + 19 * 320 + 227 + (t2 % 10) * 6,
		   corner[0] + 186 * xbuf + 8 + 228 + 19, 6, 5);
      copy_rect_2 (jukebox_img.buffer + 19 * 320 + 227 + (t2 / 10) * 6,
		   corner[0] + 186 * xbuf + 8 + 228 + 13, 6, 5);
      if (dp == 0)
	copy_rect_2 (jukebox_img.buffer + 19 * 320 + 227 + 60 - 1,
		     corner[0] + 186 * xbuf + 8 + 228 + 10, 2, 5);
      copy_rect_2 (jukebox_img.buffer + 19 * 320 + 227 + (t % 10) * 6,
		   corner[0] + 186 * xbuf + 8 + 227 + 6, 6, 5);
      copy_rect_2 (jukebox_img.buffer + 19 * 320 + 227 + (t / 10) * 6,
		   corner[0] + 186 * xbuf + 8 + 227, 6, 5);

      {
	int lempos = read_htimer (lemming_htimer);
	copy_rect_transp (main_font_img.buffer +
			  81 * 320 + 132 + 6 * (lempos & 7),
			  corner[0] + (190) * xbuf + (lempos / 2) - 6, 6, 10);
	if ((lempos / 2) >= 332)
	  reset_htimer (lemming_htimer);
      }

      vsynch ();
      aff_buffer ();
    } while (!key_or_joy_ready ());
    k = get_key_or_joy ();
    if (k == HK_Up || k == HK_Down || k == HK_Left || k == HK_Right)
      event_sfx (79);
    if (k == HK_Up || k == HK_Left) {
      if (l > 0)
	l--;
      else
	l = 2;
    }
    if (k == HK_Down || k == HK_Right) {
      if (l < 2)
	l++;
      else
	l = 0;
    }
    if (k == HK_Enter) {
      if (l == 2)
	k = HK_Escape;
      else {
	if (l == 0) {
	  event_sfx (74);
	  soundtrack_current_nbr++;
	  if (soundtrack_current_nbr == 11)
	    soundtrack_current_nbr = 0;
	};
	if (l == 1) {
	  event_sfx (75);
	  soundtrack_current_nbr--;
	  if (soundtrack_current_nbr == -1)
	    soundtrack_current_nbr = 10;
	};
	unload_soundtrack ();
	load_soundtrack_from_alias (soundtrack_list[soundtrack_current_nbr]);
	play_soundtrack ();
	reset_htimer (sound_track_htimer);
      }
    }
  } while (k != HK_Escape);
  event_sfx (76);
  in_jokebox = 0;
  free_htimer (lemming_htimer);
}

static void
load_level_from_number (int nbr, char cont)
{
  char tmp[1024];
  char e;

  dmsg (D_SECTION, "load level #%d", nbr);

  {
    char* t = get_non_null_rsc_file ("levels-dir");
    strcat (strcpy ((char *) tmp, t), level_list + nbr * levellstchunk);
    free (t);
  }
  e = load_level ((char *) tmp, cont);
  if (e != 0) {
    emsg ("Error %d during loading level", e);
  }
}

/*
static void loadlvlpasrandq2(int nbr,char cont)
{
  char tmp[1024];char e;
  strcat(strcpy((char*)tmp,nivdir),levellstq2[nbr]);
  e=load_level((char*)tmp,cont);
  if (e!=0) {
    sprintf(tmp,"Error %d during loading level\n",e);
    fatal_error((char*)tmp);
  }
}
*/
static void
compute_level_full_list (void)
{
  int i;
  unsigned int j;

  dmsg (D_SECTION, "compute level full list");

  level_full_list_size = level_list_nbr + extra_nbr;
  level_full_list = malloc (level_full_list_size * sizeof (int));
  assert (level_full_list != NULL);

  if (extra_nbr == 0) {
    extrasel = 0;
    opt.extras = 0;
  }
  if (extrasel) {
    i = extra_nbr - 1;
    while ((i >= 0) && extra_selected_list[i] == 0)
      i--;
    if (i < 0)
      opt.extras = 0;
      /* printf("%d\n",i); */
  }
  i = 0;
  if (opt.extras != 2)
    for (; i < level_list_nbr; i++)
      level_full_list[i] = i;
  if (opt.extras != 0)
    for (j = 0; j < extra_nbr; j++)
      if (extra_selected_list[j] || (extrasel == 0))
	level_full_list[i++] = (j | 0x10000);
  level_full_list_size = i;
}
static void
free_level_full_list (void)
{
  free (level_full_list);
}

static int
random_level (void)
{
  int i, j, k;
  assert (level_full_list_size > 0);
  i = rand () % level_full_list_size;
  level_full_list_size--;
  k = level_full_list[i];
  for (j = i; j < level_full_list_size; j++)
    level_full_list[j] = level_full_list[j + 1];
  return (k);
}

static void
load_random_wrapped_level (char c, char cont)
{
  int t;
  char tmp[1024];
  char e;

  dmsg (D_SECTION, "load random wrapped level");

  do {
    t = rand () % level_list_nbr;
  } while (c == 1 && levelinf[t] != 1);
  {
    char* tt = get_non_null_rsc_file ("levels-dir");
    strcat (strcpy ((char *) tmp, tt), level_list + t * levellstchunk);
    free (tt);
  }
  e = load_level ((char *) tmp, cont);
  if (e != 0) {
    emsg ("Error %d during loading level", e);
  }
}

static void
load_random_level (char cont)
{
  int t;
  char tmp[1024];
  char e;
  t = random_level ();
  /* t=1; */

  dmsg (D_SECTION, "load random wrapped");

  if (t & 0x10000) {
    strcpy (tmp, extra_list[t & 0xffff].full_name);
  } else {
    char* tt = get_non_null_rsc_file ("levels-dir");
    strcat (strcpy ((char *) tmp, tt), level_list + t * levellstchunk);
    free (tt);
  }
  e = load_level ((char *) tmp, cont);
  if (e != 0) {
    emsg (tmp, "Error %d during loading level", e);
  }

}

static void
enter_your_name (char c, char* name)
{
  keycode_t t = 0;
  int pos = 0;
  char l;
  char head[256];
  htimer_t pixelize_timer = new_htimer (T_GLOBAL, HZ (7));

  memset (name, 0, PLAYER_NAME_SIZE + 1);

  std_white_fadein (&tile_set_img.palette);
  event_sfx (73);
  do {
    background_menu ();
    sprintf (head, txti[35], c);
    draw_text_waving (head, 159, 20, 1);
    draw_text (txti[36], 159, 40, 1);
    draw_text (txti[37], 159, 70, 1);
    draw_text (name, 159, 120, 1);
    copy_rect_transp (main_font_img.buffer + 61 * 320,
		      corner[0] + 112 * xbuf + 100, 120, 3);
    copy_rect_transp (main_font_img.buffer + 61 * 320,
		      corner[0] + 135 * xbuf + 100, 120, 3);

    vsynch ();
    {
      long p = read_htimer (pixelize_timer);
      if (p <= 6)
	pixelize[6 - p] (screen, corner[0]);
      else
	aff_buffer ();
    }
    if (key_ready ()) {
      t = get_key ();
      l = t & 255;
      if (l >= 'a' && l <= 'z')
	l -= 'a' - 'A';
      if (pos < PLAYER_NAME_SIZE)
	if ((l > 20 && l <= 95) || (l == 20 && pos != 0)) {
	  name[pos] = l;
	  pos++;
	  name[pos] = 0;
	  event_sfx (70);
	}
      if ((t == HK_BackSpace || t == HK_Delete) && (pos > 0)) {
	pos--;
	name[pos] = 0;
	event_sfx (71);
      }
    }
  } while (t != HK_Escape && t != HK_Enter);
  event_sfx (72);
  if (pos == 0 || t == HK_Escape)
    memset (name, 0, PLAYER_NAME_SIZE);
  free_htimer (pixelize_timer);
}

static void
play_menu (void)
{
  char cont;
  FILE *ftmp;
  int gamemodeh;
  static int l = 1, u = 0;
  char flagload = 0;
  keycode_t t;
  int i;
  int mag;
  htimer_t flip_timer;
  long flip_pos;

  if (l == 5)
    l = 1;

  for (i = 3; i >= 0; i--) {
    col2plr[i] = opt.player_color[i];
    plr2col[opt.player_color[i]] = i;
  }

  std_white_fadein (&tile_set_img.palette);
  do {
    do {
      draw_play_menu (l);
      vsynch ();
      aff_buffer ();
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
	if (t == HK_Up || HK_Down)
	  event_sfx (1);
	if (t == HK_Up) {
	  if (l > 0)
	    l--;
	  else
	    l = 7;
	}
	if (t == HK_Down) {
	  if (l < 7)
	    l++;
	  else
	    l = 0;
	}
	if (t == HK_Escape) {
	  if (l != 7) {
	    l = 7;
	    event_sfx (1);
	  } else
	    t = HK_Enter;
	}
      } else
	t = 0;
    } while (t != HK_Enter);
    if (l == 0) {
      event_sfx (5);
      two_players ^= 1;
    }
    if (l == 7) {
      event_sfx (8);
      return;
    }
    if (l == 6) {
      std_white_fadein (&tile_set_img.palette);
      do {
	background_menu ();
	draw_saved_games_info (0, u, 0);
	vsynch ();
	aff_buffer ();
	if (key_or_joy_ready ()) {
	  t = get_key_or_joy ();
	  if (t == HK_Up) {
	    if (u > 0)
	      u--;
	    else
	      u = 9;
	  }
	  if (t == HK_Down) {
	    if (u < 9)
	      u++;
	    else
	      u = 0;
	  }
	} else
	  t = 0;
	if (t == HK_Enter && saverec[u].used == 0) {
	  event_sfx (18);
	  t = 0;
	}
      } while (t != HK_Enter && t != HK_Escape);
      if (t == HK_Escape) {
	event_sfx (8);
      }
      if (t == HK_Enter)
	flagload = 1;
    }
  } while (l == 0 || /*l==5 || l==6 || */ (l == 6 && flagload == 0));
  event_sfx (9);

  flip_timer = new_htimer (T_GLOBAL, HZ (280));
  do {
    if (flagload == 0)
      draw_play_menu (l);
    else {
      background_menu ();
      draw_saved_games_info (0, u, 0);
    }
    flip_pos = -read_htimer (flip_timer);
    if (flip_pos < -256)
      flip_pos = -256;
    flip_buffer (flip_pos);
    vsynch ();
    display_buffer_tmp1 ();
  } while (flip_pos > -256);
  vsynch ();
  /* erase the one-line flipped screen */
  memset (screen + 100 * 320, 0, 320);
  vsynch ();

  free_htimer (flip_timer);

  if (flagload == 0) {
    if (l == 1 )
      game_mode = M_QUEST;
    else if (l == 2)
      game_mode = M_KILLEM;
    else if (l == 3)
      game_mode = M_DEATHM;
    else if (l == 4)
      game_mode = M_TCASH;
    else if (l == 5)
      game_mode = M_COLOR;
    gamemodeh = game_mode;
    cont = 0;
    current_quest_level = 0;
    game_magic = compute_magic ();
  } else {
    gamemodeh = game_mode = M_QUEST;
    current_quest_level = saverec[u].level;
    game_magic = saverec[u].magic;
    for (t = 0; t < 4; t++) {
      player[col2plr[t]].lifes = saverec[u].lifes[t];
      player[col2plr[t]].score = saverec[u].points[t];
    }
    cont = 1;
  }

  unload_level ();		/* also stop the music */

  if (opt.sfx)
    load_sfx_mode (game_mode);
  if (game_mode > M_QUEST) {
    compute_level_full_list ();
    rounds = rounds_nbr_values[opt.gamerounds];
  } else
    rounds = 1;
  while (((current_quest_level < level_list_nbr)
	  || ((game_mode > M_QUEST)
	      && (current_quest_level < level_full_list_size)))
	 && (rounds > 0) && (play_game (cont) == 0)) {
    cont = 1;
    if (game_mode > M_QUEST) {
      rounds--;
      if (level_full_list_size == 0)
	rounds = 0;
    }
  }

  if (game_mode > M_QUEST)
    free_level_full_list ();

  /* END OF THE GAME */

  if ((game_mode == M_QUEST) && (current_quest_level >= level_list_nbr)) {
    if (level_is_finished != 15) {
      /* End scroller */
      char* tmp;
      char* tmp2;
      dmsg (D_SECTION, "-- end scroller --");
      tmp = get_non_null_rsc_file ("tiles-sets-dir");
      tmp2 = strcat_alloc (tmp, "level02.glz");
      free (tmp);
      ftmp = fopen (tmp2, "rb");
      free (tmp2);
      fread (glenz, 256, 8, ftmp);
      fclose (ftmp);

      load_soundtrack_from_alias ("ENDSCROLL");
      play_soundtrack ();
      end_scroll ();
      unload_soundtrack ();
    }
  }
/* ************ */

  dmsg (D_SECTION, "-- (back to) menu (from game) --");

  game_mode = M_QUEST;
/* free_all_sfx(); */
  load_random_wrapped_level (1, cont);
  load_sfx_mode (-1);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  for (t = 0; t < 4; t++)
    if (player[t].cpu == 2) {
      mag = find_magic (game_magic);
      if (mag == -1)
	mag = 9;
      if (highs[gamemodeh][mag].points >= player[t].score)
	mag = -1;
      if (mag != -1) {
	enter_your_name (plr2col[t] + 1, highs[gamemodeh][mag].name);
	highs[gamemodeh][mag].magic = game_magic;
	highs[gamemodeh][mag].points = player[t].score;
	sort_scores ();
	write_scores ();
      }
    }
}

static void
output_screen (char n)
{
  unsigned char *src;
  int i;
  char loginf[4];

  if (game_mode == M_DEATHM)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].lifes;
  else if (game_mode == M_KILLEM)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].lemmings_nbr;
  else if (game_mode >= M_TCASH)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].cash;

  if (two_players == 0) {
    compute_corner (0, n);
    draw_level (0);
    if (player[col2plr[0]].waves) {
      wave_buffer ();
      corner[0] = render_buffer[0];
    }
    if (player[col2plr[0]].rotozoom) {
      rotozoom_buffer ();
      corner[0] = render_buffer[1] + xbuf;
    }
    if (opt.radar_map)
      draw_radar_map (player[col2plr[0]].x2, player[col2plr[0]].y2);
    if (opt.display_infos)
      draw_score (col2plr[0], 0, 5 + 5 * xbuf - radar_current_pos);
    if (game_mode != M_QUEST) {
      if (radar_current_pos <= 70)
	draw_logo_info (col2plr[0], loginf[0],
			corner[0] + (183 + ((radar_current_pos >= 30)
					    ? ((radar_current_pos - 30) >> 1)
					    : 0)) * xbuf + 24);
      if (radar_current_pos <= 60)
	draw_logo_info (col2plr[1], loginf[1],
			corner[0] + (183 + ((radar_current_pos >= 20)
					    ? ((radar_current_pos - 20) >> 1)
					    : 0)) * xbuf + 98);
      if (radar_current_pos <= 50)
	draw_logo_info (col2plr[2], loginf[2],
			corner[0] + (183 + ((radar_current_pos >= 10)
					    ? ((radar_current_pos - 10) >> 1)
					    : 0)) * xbuf + 172);
      if (radar_current_pos <= 40)
	draw_logo_info (col2plr[3], loginf[3],
			corner[0] + (183 + (radar_current_pos >> 1)) * xbuf +
			246);
    }
    if (player[col2plr[0]].spec != 0xde)
      show_txt_bonus (col2plr[0], 0, 40, 5);
  } else {
    compute_corner (0, n);
    compute_corner (1, n);
    draw_level (0);
    draw_level (1);
    if (level_is_finished == 0) {
      if (player[col2plr[0]].waves) {
	wave_half_buffer (0);
	corner[0] = render_buffer[0];
      }
      if (player[col2plr[0]].rotozoom) {
	rotozoom_half_buffer (0);
	corner[0] = render_buffer[0] + xbuf - 180 + xbuf;
      }
      if (player[col2plr[1]].waves) {
	wave_half_buffer (1);
	corner[1] = render_buffer[1];
      }
      if (player[col2plr[1]].rotozoom) {
	rotozoom_half_buffer (1);
	corner[1] = render_buffer[1] + xbuf - 180 + xbuf;
      }
    }
    if (game_mode != M_QUEST) {
      if (radar_current_pos <= 60)
	draw_logo_info (col2plr[0], loginf[0],
			corner[swapside] + (5 - ((radar_current_pos >= 30)
						 ? ((radar_current_pos - 30)
						    >> 1) : 0)) * xbuf + 44);
      if (radar_current_pos <= 50)
	draw_logo_info (col2plr[1], loginf[1],
			corner[swapside] + (5 - ((radar_current_pos >= 20)
						 ? ((radar_current_pos - 20)
						    >> 1) : 0)) * xbuf + 104);
      if (radar_current_pos <= 40)
	draw_logo_info (col2plr[2], loginf[2],
			corner[1 - swapside] + (5 - ((radar_current_pos >= 10)
						     ? (
							(radar_current_pos -
							 10) >> 1) : 0)) *
			xbuf + 6);
      if (radar_current_pos <= 30)
	draw_logo_info (col2plr[3], loginf[3],
			corner[1 - swapside] + (5 -
						(radar_current_pos >> 1)) *
			xbuf + 66);
    }
    if (player[col2plr[0]].spec != 0xde)
      show_txt_bonus (col2plr[0], 0, 1, 185);
    if (player[col2plr[1]].spec != 0xde)
      show_txt_bonus (col2plr[1], 1, 2, 185);

    src = corner[swapside] + 158;
    for (i = 200; i != 0; i--) {
      *src++ = glenz[0][(int) *src];
      *src = glenz[0][(int) glenz[0][(int) *src]];
      src += xbuf - 1;
    }

    src = corner[1 - swapside];
    for (i = 200; i != 0; i--) {
      *src++ = glenz[0][(int) glenz[0][(int) *src]];
      *src = glenz[0][(int) *src];
      src += xbuf - 1;
    }

    if (opt.display_infos) {
      if (swapside) {
	draw_score (col2plr[0], 0, 122 + 5 * xbuf + radar_current_pos);
	draw_score (col2plr[1], 1, 5 + 5 * xbuf - radar_current_pos);
      } else {
	draw_score (col2plr[0], 0, 5 + 5 * xbuf - radar_current_pos);
	draw_score (col2plr[1], 1, 122 + 5 * xbuf + radar_current_pos);
      }
    }
  }
}

/****************** Amortized pendulum *********************/
/* solved with Euler's method */
double theta;			/* angle */
double theta_prime;		/* speed */
#define EULER_STEP 0.006
#define NBR_STEPS 10
int elapsed_time;		/* number of step performed so far */

static void
pendulum_init (void)
{
  theta = 3.1415926535 / 2;
  theta_prime = 0;
  elapsed_time = 0;
}

static void
pandulum_one_step (void)
{
  double aux;
  aux = EULER_STEP * (0.33 * theta_prime + sin (theta));
  theta += EULER_STEP * theta_prime;
  theta_prime -= aux;
  elapsed_time += 1;
}

static int
pendulum_update (int n)
{
  int i;
  for (i = NBR_STEPS * n; i != 0; --i)
    pandulum_one_step ();
  return (floor (theta * 512.0 / 3.1415926535));
}

/****************** ******* ****** *********************/
void
grow_trail (int pl, int size)
{
  int i, k;

  k = ((trail_offset[pl] + trail_size[pl] - 1) & (maxq - 1));
  while (size != 0 && trail_size[pl] + 5 < maxq) {
    i = ((trail_offset[pl] + trail_size[pl]) & (maxq - 1));
    trail_pos[pl][i] = trail_pos[pl][k];
    trail_way[pl][i] = trail_way[pl][k];
    trail_size[pl]++;
    --size;
  }
  if (trail_size[pl] >= 55 && player[pl].cpu == 2 && game_mode == M_QUEST)
    event_sfx (89);
  if (trail_size[pl] >= 55 && game_mode == M_QUEST)
    add_end_level_bonuses ();
}

void
shrink_trail (int pl, int size)
{
  int i;

  while (size != 0 && trail_size[pl] > 5) {
    --trail_size[pl];
    i = ((trail_offset[pl] + trail_size[pl]) & (maxq - 1));
    square_occupied[trail_pos[pl][i]] = 0xff;
    i = ((trail_offset[pl] + trail_size[pl] - 1) & (maxq - 1));
    square_occupied[trail_pos[pl][i]] = (unsigned char) (pl + 12);
    --size;
  }
}

static void
erase_trail (int c)
{
  int i;

  for (i = map_info.xt * map_info.yt * 4 - 1; i >= 0; i--)
    if ((square_occupied[i] & 3) == c && square_occupied[i] < 16) {
      square_occupied[i] = 0xff;
      square_dead_explosion[i] = event_time;
    }
  last_explo = event_time;
}

/****************/
static unsigned int
ia_eval_dist (int pos)
{
  int curx, cury, distx, disty;
  curx = square_offset2coord[pos << 1];
  cury = square_offset2coord[(pos << 1) + 1];
  if (ia_wrap_left) {
    if (curx <= ia_wrap_x)
      distx = curx + (map_info_2xt) - ia_target_x;
    else if (curx <= ia_target_x)
      distx = ia_target_x - curx;
    else
      distx = curx - ia_target_x;
  } else {
    if (curx >= ia_wrap_x)
      distx = ia_target_x + (map_info_2xt) - curx;
    else if (curx <= ia_target_x)
      distx = ia_target_x - curx;
    else
      distx = curx - ia_target_x;
  }
  if (ia_wrap_right) {
    if (cury <= ia_wrap_y)
      disty = cury + (map_info_2yt) - ia_targer_y;
    else if (cury <= ia_targer_y)
      disty = ia_targer_y - cury;
    else
      disty = cury - ia_targer_y;
  } else {
    if (cury >= ia_wrap_y)
      disty = ia_targer_y + (map_info_2yt) - cury;
    else if (cury <= ia_targer_y)
      disty = ia_targer_y - cury;
    else
      disty = cury - ia_targer_y;
  }
  return (distx + disty);
}

/* used by ia_goto_target
 */

#define ia_eval_dir_target_inline(s_sens)				  \
    d=square_wrap[(pos<<2)+s_sens];					  \
    if (d!=-1)								  \
    if ((square_occupied[d]==0xff) &&					  \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) { \
	    tmp=ia_eval_dir_target(d);					  \
	    if (tmp<mindist) mindist=tmp;				  \
    }

#define ia_eval_dir_bonus_inline(s_sens)				  \
    d=square_wrap[(pos<<2)+s_sens];					  \
    if (d!=-1)								  \
    if ((square_occupied[d]==0xff) &&					  \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) { \
	    tmp=ia_eval_dir_bonus(d);					  \
	    if (tmp>mindist) mindist=tmp;				  \
    }

#define ia_eval_dir_lemming_inline(s_sens)				  \
    d=square_wrap[(pos<<2)+s_sens];					  \
    if (d!=-1)								  \
    if ((square_occupied[d]==0xff) &&					  \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) { \
	    tmp=ia_eval_dir_lemming(d);					  \
	    if (tmp>mindist) mindist=tmp;				  \
    }

#define ia_eval_dir_cash_inline(s_sens)					  \
    d=square_wrap[(pos<<2)+s_sens];					  \
    if (d!=-1)								  \
    if ((square_occupied[d]==0xff) &&					  \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) { \
	    tmp=ia_eval_dir_cash(d);					  \
	    if (tmp>mindist) mindist=tmp;				  \
    }

#define ia_eval_dir_color_inline(s_sens)				  \
    d=square_wrap[(pos<<2)+s_sens];					  \
    if (d!=-1)								  \
    if ((square_occupied[d]==0xff) &&					  \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) { \
	    tmp=ia_eval_dir_color(d);					  \
	    if (tmp>mindist) mindist=tmp;				  \
    }

/*
-> maluses in the position evaluation function
neighb wall ..... : 2pts,
neighb enemy .... : 5pts,
self (old) ...... : 2pts,
self (new) (128). : 1pts,
4pts count as one square in the distance function
*/

static int
ia_eval_neighb_pos (char s_sens, int pos)
{
  int d;
  unsigned char c;
  d = square_wrap[(pos << 2) + s_sens];
  if (d != -1) {
    c = square_occupied[d];
    if (c < 128) {
      if ((c & 3) == ia_player)
	return (2);
      else
	return (5);
    } else if (c == 128)
      return (1);
    else
      return (0);
  }
  return (2);
}

static unsigned int
ia_eval_dir_target (int pos)
{
  u32_t mindist;
  int d;
  unsigned int tmp;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_occupied[pos] = 128;
    mindist = U32_MAX;

    ia_eval_dir_target_inline (w_up);
    ia_eval_dir_target_inline (w_right);
    ia_eval_dir_target_inline (w_down);
    ia_eval_dir_target_inline (w_left);

    square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return (mindist);
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return ((ia_eval_dist (pos) << 16) + (tmp << 14) + ia_max_depth -
	    ia_cur_depth);
  }

}

static int
ia_eval_dir_lemming (int pos)
{
  int mindist, d;
  int tmp, tmp2;
  lemming_t *tmppti;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_occupied[pos] = 128;
    mindist = 0;
    tmppti = square_lemmings_list[pos];
    if (tmppti >= lemmings_support
	&& tmppti < (lemmings_support + lemmings_total)) {
      if (tmppti->couleur == ia_player)
	tmp2 = -100;
      else
	tmp2 = 20;
    } else
      tmp2 = 0;

    ia_eval_dir_lemming_inline (w_up);
    ia_eval_dir_lemming_inline (w_right);
    ia_eval_dir_lemming_inline (w_down);
    ia_eval_dir_lemming_inline (w_left);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return (mindist);
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return (-(tmp << 2));
  }

}

static int
ia_eval_dir_color (int pos)
{
  signed int mindist, d;
  int tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_occupied[pos] = 128;
    mindist = 0;
    d = (signed int) square_object[pos];
    tmp2 = 0;
    if (d >= 0) {
      if (d <= 4) {
	if (d == ia_player)
	  tmp2 = 100;
	else
	  tmp2 = -200;
      } else if (d <= 12) {
	if (d == ia_player + 8)
	  tmp2 = -200;
	else
	  tmp2 = 100;
      } else if (d == 16)
	tmp2 = 100;
      else if (d == 24)
	tmp2 = -40;
    }

    ia_eval_dir_color_inline (w_up);
    ia_eval_dir_color_inline (w_right);
    ia_eval_dir_color_inline (w_down);
    ia_eval_dir_color_inline (w_left);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return (mindist);
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return (-(tmp << 2));
  }

}

static int
ia_eval_dir_cash (int pos)
{
  signed int mindist, d;
  int tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_occupied[pos] = 128;
    mindist = 0;
    d = (signed int) square_object[pos];

    if (d >= 0)
      tmp2 = 500;
    else
      tmp2 = -20;

    ia_eval_dir_cash_inline (w_up);
    ia_eval_dir_cash_inline (w_right);
    ia_eval_dir_cash_inline (w_down);
    ia_eval_dir_cash_inline (w_left);

    mindist += tmp2;		/* *(5+ia_cur_depth); */

    square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return (mindist);
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return (-(tmp << 2));
  }

}

static int
ia_eval_dir_bonus (int pos)
{
  int mindist, d;
  int tmp, tmp2;

  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    square_occupied[pos] = 128;
    mindist = 0;
    d = square2tile[pos];
    tmp2 = 0;
    if (tile_bonus_cpu[d] == 0) {
      tmp = tile_bonus[d];
      if ((tmp != 0) && (tmp != 0xff)) {
	tile_bonus_cpu[d] = 1;
	if (tmp < 128)
	  tmp2 = bonus_points[0][tmp - 1];
	else
	  tmp2 = bonus_points[1][tmp - 129];
      }
    }
    ia_eval_dir_bonus_inline (w_up);
    ia_eval_dir_bonus_inline (w_right);
    ia_eval_dir_bonus_inline (w_down);
    ia_eval_dir_bonus_inline (w_left);

    mindist += tmp2 * (5 + ia_cur_depth) /* /ia_max_depth */ ;

    if (tmp2)
      tile_bonus_cpu[square2tile[pos]] = 0;
    square_occupied[pos] = 0xff;
    ia_cur_depth++;
    return (mindist);
  } else {
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return (-(tmp << 3));
  }

}

/* give the *way* to follow to get a given position */

#define ia_goto_target_inline(s_sens)                              \
    d=square_wrap[(pos<<2)+s_sens];                              \
    if (d!=-1)                                                \
    if ((square_occupied[d]==0xff) &&                               \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) {  \
	  ia_cur_depth=ia_max_depth;                              \
	  tmp[s_sens]=ia_eval_dir_target(d);                             \
	  if (tmp[s_sens]<mindist) {                          \
		mindist=tmp[s_sens];                          \
		mindir=s_sens;                                \
	  }                                                   \
    }

#define ia_goto_bonus_inline(s_sens)                               \
    d=square_wrap[(pos<<2)+s_sens];                              \
    if (d!=-1)                                                \
    if ((square_occupied[d]==0xff) &&                               \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) {  \
	  ia_cur_depth=ia_max_depth;                              \
	  tmp[s_sens]=ia_eval_dir_bonus(d);                        \
	  if (tmp[s_sens]>mindist) {                          \
		mindist=tmp[s_sens];                          \
		mindir=s_sens;                                \
	  }                                                   \
    }

#define ia_goto_lemming_inline(s_sens)                              \
    d=square_wrap[(pos<<2)+s_sens];                              \
    if (d!=-1)                                                \
    if ((square_occupied[d]==0xff) &&                               \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) {  \
	  ia_cur_depth=ia_max_depth;                              \
	  tmp[s_sens]=ia_eval_dir_lemming(d);                       \
	  if (tmp[s_sens]>mindist) {                          \
		mindist=tmp[s_sens];                          \
		mindir=s_sens;                                \
	  }                                                   \
    }

#define ia_goto_color_inline(s_sens)                               \
    d=square_wrap[(pos<<2)+s_sens];                              \
    if (d!=-1)                                                \
    if ((square_occupied[d]==0xff) &&                               \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) {  \
	  ia_cur_depth=ia_max_depth;                              \
	  tmp[s_sens]=ia_eval_dir_color(d);                        \
	  if (tmp[s_sens]>mindist) {                          \
		mindist=tmp[s_sens];                          \
		mindir=s_sens;                                \
	  }                                                   \
    }

#define ia_goto_cash_inline(s_sens)                              \
    d=square_wrap[(pos<<2)+s_sens];                              \
    if (d!=-1)                                                \
    if ((square_occupied[d]==0xff) &&                               \
       ((square_explosion[d]>=(nfrexplo1-1)*8+12) || ia_is_invincible)) {  \
	  ia_cur_depth=ia_max_depth;                              \
	  tmp[s_sens]=ia_eval_dir_cash(d);                       \
	  if (tmp[s_sens]>mindist) {                          \
		mindist=tmp[s_sens];                          \
		mindir=s_sens;                                \
	  }                                                   \
    }

static char
ia_goto_target (int c, int targetx_, int targety_)
{
  int d, pos;
  u32_t tmp[4] = { U32_MAX, U32_MAX, U32_MAX, U32_MAX };
  u32_t mindist = U32_MAX;
  int mindir = 0;

  ia_player = c;
  ia_max_depth = player[c].ia_max_depth;
  ia_target_x = targetx_;
  ia_targer_y = targety_;
  ia_wrap_x = ia_target_x + map_info.xt;
  if (ia_wrap_x >= (int)map_info_2xt) {
    ia_wrap_x -= map_info_2xt;
    ia_wrap_left = 1;
  } else
    ia_wrap_left = 0;
  ia_wrap_y = ia_targer_y + map_info.yt;
  if (ia_wrap_y >= (int)map_info_2yt) {
    ia_wrap_y -= map_info_2yt;
    ia_wrap_right = 1;
  } else
    ia_wrap_right = 0;

  ia_is_invincible = (player[c].invincible != 0);
  pos = player[c].pos;
  ia_goto_target_inline (w_up);
  ia_goto_target_inline (w_right);
  ia_goto_target_inline (w_down);
  ia_goto_target_inline (w_left);
  if (tmp[player[c].way] == tmp[mindir])
    return (player[c].way);
  else
    return (mindir);
}

static char
ia_goto_nearest_bonus (int c)
{
  int d, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  int mindir = 0;

  ia_player = c;
  ia_max_depth = player[c].ia_max_depth;
  ia_is_invincible = (player[c].invincible != 0);
  pos = player[c].pos;
  ia_goto_bonus_inline (w_up);
  ia_goto_bonus_inline (w_right);
  ia_goto_bonus_inline (w_down);
  ia_goto_bonus_inline (w_left);
  if (tmp[player[c].way] == tmp[mindir])
    return (player[c].way);
  else
    return (mindir);
}

static char
ia_goto_nearest_lemming (int c)
{
  int d, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  int mindir = 0;

  ia_player = c;
  ia_max_depth = player[c].ia_max_depth;
  ia_is_invincible = (player[c].invincible != 0);
  pos = player[c].pos;
  ia_goto_lemming_inline (w_up);
  ia_goto_lemming_inline (w_right);
  ia_goto_lemming_inline (w_down);
  ia_goto_lemming_inline (w_left);
  if (tmp[player[c].way] == tmp[mindir])
    return (player[c].way);
  else
    return (mindir);
}

static char
ia_goto_nearest_color (int c)
{
  int d, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  int mindir = 0;

  ia_player = c;
  ia_max_depth = player[c].ia_max_depth;
  ia_is_invincible = (player[c].invincible != 0);
  pos = player[c].pos;
  ia_goto_color_inline (w_up);
  ia_goto_color_inline (w_right);
  ia_goto_color_inline (w_down);
  ia_goto_color_inline (w_left);
  if (tmp[player[c].way] == tmp[mindir])
    return (player[c].way);
  else
    return (mindir);
}

static char
ia_goto_nearest_cash (int c)
{
  int d, pos;
  int tmp[4] = { 0, 0, 0, 0 };
  int mindist = 0;
  int mindir = 0;

  ia_player = c;
  ia_max_depth = player[c].ia_max_depth;
  ia_is_invincible = (player[c].invincible != 0);
  pos = player[c].pos;
  ia_goto_cash_inline (w_up);
  ia_goto_cash_inline (w_right);
  ia_goto_cash_inline (w_down);
  ia_goto_cash_inline (w_left);
  if (tmp[player[c].way] == tmp[mindir])
    return (player[c].way);
  else
    return (mindir);
}

/****************/

/* search for a free direction */
static void
find_free_way (int c)
{
  int d = 0, n = 0, o[4] = { 0xff, 0xff, 0xff, 0xff }, e, f;
  int i, m;

  m = (player[c].x2 + player[c].y2 * (map_info_2xt)) << 2;
  e = 1;
  for (i = 0; i < 4; i++) {
    if (square_wrap[m + i] != -1)
      o[i] = square_occupied[square_wrap[m + i]];
    if (o[i] != 0xff || square_wrap[m + i] == -1)
      d |= e;
    e += e;
  }

/* if (player[c].spec==t_tunnel*8) {player[c].spec=0;* return;*} */
/* if (player[c].spec==t_tunnel*4) {player[c].spec=t_tunnel*8; return;} */


  f = player[c].next_way;
  if (!(d & (1 << f)))
    return;
  e = o[player[c].next_way];

  /* when a trail force someone to turn, the owner of this trail is credited */
  if ((e & 3) != c && (!level_is_finished) && e != 0xff
      && (player[e & 3].spec != 0xde))
    player[e & 3].score += 5;

  if (!(d & (1 << player[c].old_old_way))) {
    player[c].next_way = player[c].old_old_way;
    return;
  }

  if (!(d & (1 << player[c].old_way))) {
    player[c].next_way = player[c].old_way;
    return;
  }

  if (player[c].spec != t_ice)
    for (i = 1; i != 16; i += i)
      if (!(d & i))
	n++;

  e = d;
  if (n != 0) {
    n = (char) (1 + rand () % n);
    for (i = 0; n != 0; i++, d >>= 1)
      if (!(d & 1))
	n--;
    player[c].next_way = (char) (i - 1);
/*  if (w2d[i-1]&e) fatal_error("find_free_way() return nonsense !"); */
    assert ((w2d[i - 1] & e) == 0);
  } else
    player[c].spec = 0xff;
}

/* * * * * * * * * * * * * * * * * * * * *\
 * Handling of players moves, collisions, *
 * bonus effects, lemmings dies, etc.     *
 \* * * * * * * * * * * * * * * * * * * **/

static void
update_player (int c)
{
  int a, b, d, e, l, i;
  unsigned int d2;
  int t;
  lemming_t *tmppti;

  if ((player[c].score_delta >> 2) < player[c].score) {
    player[c].score_delta++;
    /* 1 life every 10.000 points */
    if (player[c].score_delta % (10000 << 2) == 0)
      apply_bonus (c, 15);
  }
/* if ((player[c].score_delta>>2)>player[c].score) player[c].score_delta--; */
  if (player[c].turbo_level_delta < player[c].turbo_level) {
    player[c].turbo_level_delta += 8;
    if (player[c].turbo_level_delta > player[c].turbo_level)
      player[c].turbo_level_delta = player[c].turbo_level;
  } else if (player[c].turbo_level_delta > player[c].turbo_level) {
    player[c].turbo_level_delta -= 8;
    if (player[c].turbo_level_delta < player[c].turbo_level)
      player[c].turbo_level_delta = player[c].turbo_level;
  }

  if (player[c].invincible > 0)
    player[c].invincible--;
  if (game_mode >= M_TCASH) {
    if (player[c].time > 0) {
      player[c].time--;
/* stop the game if the player is alone */
/*       if ((!level_is_finished) && */
/*           (player[(c+1)&3].spec==0xde) && */
/*           (player[(c+2)&3].spec==0xde) && */
/*           (player[(c+3)&3].spec==0xde)) { level_is_finished=c+1; return; } */
    } else if (!level_is_finished) {
      player[c].spec = 0xde;
      erase_trail (c);
      /* stop the game if all human players are dead or
	 if there is no more colors or dollars */
      if ((!(((player[0].cpu & 2) && (player[0].time))
	     || ((player[1].cpu & 2) && (player[1].time))
	     || ((player[2].cpu & 2) && (player[2].time))
	     || ((player[3].cpu & 2) && (player[3].time))))
	  || (objects_nbr == 0)) {
	level_is_finished = 0;
	for (i = 1; i < 4; i++)
	  if (player[i].cash > player[level_is_finished].cash)
	    level_is_finished = i;
	level_is_finished++;
      }
    }
  }
  update_player_bonus_vars (c);
  d = (player[c].x2 >> 1) + (player[c].y2 >> 1) * map_info.xt;
  if (player[c].rotozoom != 0)
    player[c].rotozoom--;
  if (player[c].waves != 0) {
    player[c].waves--;
    if (player[c].waves > 128 && player[c].waves_begin < 128)
      player[c].waves_begin++;
    if (player[c].waves < 128 && player[c].waves_begin > 0)
      player[c].waves_begin--;
  }

  if (player[c].spec == 0xde)
    return;

  if (player[c].inversed_controls > 0) {
    player[c].inversed_controls--;
    sprintf (txt_tmp, txti[24], player[c].inversed_controls / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }

  if (player[c].delay == 0) {
    if (player[c].turbo != 1 && player[c].turbo_level > 0
	&& player[c].speedup == 0) {
      player[c].vitt = (player[c].v + player[c].vi) * player[c].turbo;
      player[c].turbo_level -= 2;
    } else if (player[c].speedup > 0) {
      player[c].vitt = (player[c].v + player[c].vi) << 1;
      player[c].speedup--;
    } else if (player[c].speedup < 0) {
      player[c].vitt = (player[c].v + player[c].vi) >> 1;
      player[c].speedup++;
    } else
      player[c].vitt = (player[c].v + player[c].vi);
    if (player[c].vitp < player[c].vitt)
      if (player[c].vitp + 512 < player[c].vitt)
	player[c].vitp += 512;
      else
	player[c].vitp = player[c].vitt;
    else if (player[c].vitp > player[c].vitt) {
      if (player[c].vitp + 512 > player[c].vitt)
	player[c].vitp -= 512;
      else
	player[c].vitp = player[c].vitt;
    }
    player[c].d.e += player[c].vitp;
  } else {
    player[c].delay--;
    sprintf (txt_tmp, txti[25], player[c].delay / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }


/* if (explofr[c]!=0) explofr[c]--; */
  if (player[c].d.h.h != 0 || player[c].delay == 1) {

/**** handling of trails ****/
    if ( /*player[c].spec!=t_tunnel*8 && */ player[c].delay == 0) {
      l = player[c].x2 + player[c].y2 * map_info_2xt;
      square_occupied[l] = (char) (c + 8);	/* 0xff; */
      trail_offset[c] = (char) ((trail_offset[c] - 1) & (maxq - 1));
      trail_pos[c][trail_offset[c]] = l;
      trail_way[c][trail_offset[c]] = square_way[l] =
	(char) (player[c].way + (player[c].old_way << 2));
      a = (trail_offset[c] + trail_size[c]) & (maxq - 1);
      if (trail_pos[c][a]
	  != trail_pos[c][(trail_offset[c] + trail_size[c] - 1) & (maxq - 1)]) {
	if (square_occupied[trail_pos[c][a]] == c + 12)
	  square_occupied[trail_pos[c][a]] = 0xff;
	a = (trail_offset[c] + trail_size[c] - 1) & (maxq - 1);
	if (square_occupied[trail_pos[c][a]] == c + 8)
	  square_occupied[trail_pos[c][a]] = (char) (c + 12);
      } else
	square_occupied[trail_pos[c][a]] = (char) (c + 12);

    }
/*************************/
/*     if (player[c].spec==t_tunnel) {player[c].spec=t_tunnel*4;
				    player[c].x2=(level_map[d].info.tunnel.output%map_info.xt)<<1;
				    player[c].y2=(level_map[d].info.tunnel.output/map_info.xt)<<1;
				    player[c].d.e=0;
				    if (level_map[(player[c].x2>>1)+(player[c].y2>>1)*map_info.xt].type==t_tunnel) {
					 b=level_map[(player[c].x2>>1)+(player[c].y2>>1)*map_info.xt].info.tunnel.direction;
				    } else {
					 b=w2d[d2w[level_map[d].info.tunnel.direction]^2];
				    }
				    player[c].way=d2w[b]^2;
				    player[c].next_way=d2w[b]^2;
				    if (player[i].tunnel_inverse) player[c].next_way^=2;
				    a=tunnel_square_io[d2w[b]][player[c].div];
				    if (b==d_left) player[c].x2--;
				    if (b==d_right) player[c].x2++;
				    if (b==d_up)   player[c].y2--;
				    if (b==d_down)    player[c].y2++;
				    if (a&1) player[c].x2++;
				    if (a&2) player[c].y2++;
				    }
*/

    d2 = player[c].y2 * map_info_2xt + player[c].x2;
    if (player[c].delay == 0)
      d2 = square_wrap[(d2 << 2) + player[c].way];
    player[c].pos = d2;
    player[c].x2 = square_offset2coord[d2 << 1];
    player[c].y2 = square_offset2coord[(d2 << 1) + 1];
    player[c].d.h.h = 0;

    if (player[c].spec == t_tunnel) {
      /*  player[c].old_way= */ player[c].way = player[c].tunnel_way;
      /* d2w[b]^2; */
/*             player[c].next_way=player[c].tunnel_way; d2w[b]^2;
	     if (player[i].tunnel_inverse) player[c].next_way^=2;*/
      player[c].spec = 0;
    }


    if (cpuon) {
      if (player[c].target < 16)
/*  for (i=0;i<4;i++) */
      {
	if ((player[c].cpu & 2) == 0) {
	  if (player[c].behaviour == 1)
	    player[c].next_way = ia_goto_nearest_bonus (c);
	  else if (player[c].behaviour == 2) {
	    if (game_mode == M_KILLEM)
	      player[c].next_way = ia_goto_nearest_lemming (c);
	    if (game_mode == M_TCASH)
	      player[c].next_way = ia_goto_nearest_cash (c);
	    if (game_mode == M_COLOR)
	      player[c].next_way = ia_goto_nearest_color (c);
	  } else
	    player[c].next_way =
	      ia_goto_target (c, player[player[c].target].x2,
			      player[player[c].target].y2);
	}
      } else
	player[c].target -= 16;
    }
/*       if (player[c].spec!=t_tunnel*4) */
    {
      square_occupied[d2 /*player[c].x2+player[c].y2*map_info_2xt */ ] = c;
      if (
	  (square_explosion[d2 /*player[c].x2+player[c].y2*map_info_2xt */ ]
	   <=
	   (nfrexplo1 - 1) * 8 - 1) && player[c].invincible == 0)
	player[c].spec = 0xff;
    }
    player[c].square = (char) ((player[c].x2 & 1) + (player[c].y2 & 1) * 2);
    d = (player[c].x2 >> 1) + (player[c].y2 >> 1) * map_info.xt;

/*    if (player[c].spec!=t_tunnel*4) */
    {
      if (level_map[d].type == t_ice
	  && level_map[d].info.param[player[c].square] != 0)
	player[c].spec = t_ice;
      if (
	  (level_map[d].type == t_stop
	   && level_map[d].info.param[player[c].square] != 0
	   && player[c].delay == 0) || player[c].notify_delay) {
	player[c].notify_delay = 0;
	player[c].delay = 100;
	player[c].d.e = 0;
	return;
      }

      if (game_mode == M_KILLEM) {
	tmppti = square_lemmings_list[d2];
	if (tmppti >= lemmings_support
	    && tmppti < lemmings_support + lemmings_total) {
	  if ((tmppti->pos1 == d2 && lemmings_move_offset < 38000)
	      || (tmppti->pos2 == d2 && lemmings_move_offset > 28000)) {
	    if (!level_is_finished) {
	      player[c].score += 10;
	      player[tmppti->couleur].lemmings_nbr--;
	    }
	    tmppti->dead = (rand () & 15) + 1;
	    if (rand () & 63) {
	      tmppti->couleur = 0;
	      if (player[c].cpu == 2)
		event_sfx (90 + ((tmppti->dead - 1) >> 1));
	    } else {
	      tmppti->couleur = 1;
	      if (!level_is_finished)
		player[c].score += 140;
	      player[c].martians_nbr++;
	      if (player[c].cpu == 2)
		event_sfx (98);
	    }
	    if (lemmings_move_offset < 32536) {
	      tmppti->min = lemmings_move_offset;
	      i = tmppti->pos1;
	    } else {
	      tmppti->min = 65536 - lemmings_move_offset;
	      assert (tmppti->min < 65536);
	      i = tmppti->pos2;
	      tmppti->way ^= 2;
	    }
	    square_lemmings_list[tmppti->pos1] = NULL;
	    square_lemmings_list[tmppti->pos2] = NULL;
	    tmppti->nexttache =
	      (char *) square_dead_lemmings_list[i /*tmppti->pos1 */ ];
	    square_dead_lemmings_list[i /*tmppti->pos1 */ ] = tmppti;
	  }
	}
	for (i = 0; i < 4; i++)
	  if (player[(i + 1) & 3].lemmings_nbr == 0 &&
	      player[(i + 2) & 3].lemmings_nbr == 0 &&
	      player[(i + 3) & 3].lemmings_nbr == 0)
	    level_is_finished = i + 1;
      }

      if (game_mode == M_COLOR) {
	t = square_object[d2];
	if ((signed char) t >= 0) {
	  if ((!level_is_finished)) {
	    player[c].score += 2;
	    if (player[c].cpu == 2) {
	      if (t == c)
		event_sfx (100);
	      else if (t <= 4)
		event_sfx (101);
	      if (t == 8 + c)
		event_sfx (102);
	      else if (t <= 12)
		event_sfx (103);
	      else if (t == 16)
		event_sfx (104);
	      else if (t == 24)
		event_sfx (105);
	    }
	    if ((t < 4) && (player[t].spec != 0xde))
	      player[t].cash++;
	    else if (t == 4) {
	      if (player[(c + 1) & 3].spec != 0xde)
		player[(c + 1) & 3].cash++;
	      if (player[(c + 2) & 3].spec != 0xde)
		player[(c + 2) & 3].cash++;
	      if (player[(c + 3) & 3].spec != 0xde)
		player[(c + 3) & 3].cash++;
	    } else if (t < 12) {
	      if (player[t & 3].cash > 0)
		player[t & 3].cash--;
	    } else if (t == 12) {
	      if ((player[(c + 1) & 3].cash > 0)
		  && (player[(c + 1) & 3].spec != 0xde))
		player[(c + 1) & 3].cash--;
	      if ((player[(c + 2) & 3].cash > 0)
		  && (player[(c + 2) & 3].spec != 0xde))
		player[(c + 2) & 3].cash--;
	      if ((player[(c + 3) & 3].cash > 0)
		  && (player[(c + 3) & 3].spec != 0xde))
		player[(c + 3) & 3].cash--;
	    } else if (t == 16)
	      player[c].time += 1000;
	    else if (t == 24) {
	      if (player[c].time > 333)
		player[c].time -= 333;
	      else
		player[c].time = 1;
	    }
	  }
	  square_object[d2] = -1;
	  /* add_color(0); */
	  objects_nbr--;
	}
      }

      if (game_mode == M_TCASH) {
	t = square_object[d2];
	if ((signed char) t >= 0) {
	  if (!level_is_finished) {
	    player[c].score += 2;
	    if (t == 0) {
	      player[c].cash++;
	      if (player[c].cpu == 2)
		event_sfx (80);
	    }
	    if (t == 15) {
	      player[c].time += 1000;
	      if (player[c].cpu == 2)
		event_sfx (81);
	    }
	  }
	  square_object[d2] = -1;
	  /* add_cash(0); */
	  objects_nbr--;
	}
      }

      if (game_mode == M_DEATHM) {
	for (i = 0; i < 4; i++)
	  if (player[(i + 1) & 3].lifes == 0 &&
	      player[(i + 2) & 3].lifes == 0
	      && player[(i + 3) & 3].lifes == 0) level_is_finished = i + 1;
      }
      {
	int bonus = tile_bonus[d];
	if (bonus && bonus != 0xff) {
	  rem_bonus (d);
	  if (!level_is_finished) {
	    player[c].score += 10;
	    if (bonus & 128) {
	      if (player[c].cpu == 2)
		event_sfx (39 + (bonus & 127));
	      for (i = 0; i < 4; i++)
		if ((c != i) && (player[i].spec != 0xde))
		  apply_bonus (i, (bonus & 127));
	    } else
	      apply_bonus (c, bonus);
	  }
	  if (player[c].notify_delay) {
	    player[c].notify_delay = 0;
	    player[c].delay = 100;
	    player[c].d.e = 0;
	    return;
	  }
	}
      }
    }
    player[c].old_old_way = player[c].old_way;
    player[c].old_way = player[c].way;
/*       if (player[c].spec==t_tunnel)
       { player[c].way=player[c].next_way;
	 return;
       }
*/
    if (player[c].autopilot)
      find_free_way (c);	/* here t_tunnel*4 become t_tunnel*8 !!! */
    if ((!player[c].autopilot)
	&& (player[c].next_way == (player[c].old_way ^ 2)))
      player[c].next_way = player[c].old_way;
    if (player[c].spec != t_ice)
      player[c].way = player[c].next_way;
    else {
      player[c].next_way = player[c].way;
      player[c].spec = 0;
    }

    a = square_wrap[(d2 << 2) + player[c].way];
    if (a == -1)
      player[c].spec = 0xff;
    else if (square_occupied[a] != 0xff)
      player[c].spec = 0xff;

    if (player[c].spec == 0xff) {
      if ((!level_is_finished) && player[c].invincible == 0
	  && game_mode != M_DEATHM) shrink_trail (c, 5);
      erase_trail (c);
      if (!level_is_finished)
	if (player[c].lifes == 1) {
	  player[c].lifes = 0;
	  player[c].spec = 0xde;
	  if (!(((player[0].cpu & 2) && (player[0].lifes))
		|| ((player[1].cpu & 2) && (player[1].lifes))
		|| ((player[2].cpu & 2) && (player[2].lifes))
		|| ((player[3].cpu & 2) && (player[3].lifes)))) {
	    level_is_finished = 15;
	  }
	  if (player[c].cpu == 2)
	    event_sfx (62);
	  return;
	}
      reinit_player (c);
      if (player[c].lifes != 0 && player[c].invincible == 0
	  && (!level_is_finished)) player[c].lifes--;
      if (!level_is_finished) {
	if (player[c].lifes > 1) {
	  if (player[c].cpu == 2)
	    event_sfx (60);
/*            if (player[c].lifes!=1) */
	  sprintf (txt_tmp, txti[30], player[c].lifes);
/*            else */
/*              sprintf(txt_tmp,"1 LIFE LEFT"); */

	} else {
	  sprintf (txt_tmp, txti[31]);
	  if (player[c].cpu == 2)
	    event_sfx (61);
	}
      }
      if (!level_is_finished)
	set_txt_bonus (c, txt_tmp, 150);
      player[c].invincible = 350;
      return;
    }

/******************/
    if (level_map[d].type == t_tunnel
	&& level_map[d].info.tunnel.direction == (1 << player[c].next_way))
      if (player[c].square ==
	  tunnel_square_io[d2w[level_map[d].info.tunnel.direction]][0]
	  || player[c].square ==
	  tunnel_square_io[d2w[level_map[d].info.tunnel.direction]][1]) {
	player[c].spec = t_tunnel;
	if ((player[c].cpu == 2) && (!level_is_finished))
	  event_sfx (69);
	if (level_map[level_map[d].info.tunnel.output].type == t_tunnel) {
	  b =
	    d2w[level_map[level_map[d].info.tunnel.output].info.tunnel.
		direction] ^ 2;
	} else {
	  b = d2w[level_map[d].info.tunnel.direction];
	}
	player[c].tunnel_way = (char) b;
/*             player[c].way=player[c].tunnel_way; d2w[b]^2; */
	player[c].next_way = player[c].tunnel_way;	/* d2w[b]^2; */
	if (player[c].tunnel_inverse)
	  player[c].next_way ^= 2;

      }
/*****************/



/*    if (player[c].spec!=t_tunnel*8) */
    {
      if (level_map[d].type == t_speed) {
	e = level_map[d].info.param[player[c].square];
	if ((1 << player[c].way) & e)
	  player[c].vi = player[c].v;
	else if ((1 << (player[c].way ^ 2)) & e)
	  player[c].vi = -(player[c].v >> 1);
	else
	  player[c].vi = 0;
      } else
	player[c].vi = 0;
      if (level_map[d].type == t_dust
	  && level_map[d].info.param[player[c].square] != 0)
	player[c].vi = -(player[c].v >> 1);
      if (square_explosion[d2 /*player[c].x2+player[c].y2*map_info_2xt */ ] ==
	  255) {
	square_explosion[d2 /*player[c].x2+player[c].y2*map_info_2xt */ ] =
	  200;
	square_explosion_type[d2 /*player[c].x2+player[c].y2*map_info_2xt */ ]
	  =
	  (char) (1 + (rand () & 1));
/*            return; */
      }
    }
    player[c].delay = 0;

/*      if (square_wrap[(d2<<2)+player[c].way]==-1) {printf("Le player_t %d (0-3) pénètre une dalle \"-1\" !",c); fatal_error(""); } */
    assert (square_wrap[(d2 << 2) + player[c].way] != -1);
    square_occupied[square_wrap[(d2 << 2) + player[c].way]] = (char) (c + 4);
  }
}


/* propagate explosions */

static void
update_explo (void)
{
  int i;			/* ,x,y,m,x2; */
  int x2;
  int *m;
  unsigned char c;
  for (i = explo_nbr - 1; i >= 0; i--) {
    c = *explo_list_ptr[i];
    if (c <= 200)
      *explo_list_ptr[i] = (c - 1);
    if (c == 170) {
      m = (int *) square_wrap;
      m +=
	((explo_list_pos_y[i] * (map_info_2xt)) + explo_list_pos_x[i]) << 2;
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = (char) (1 + (rand () & 1));
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = (char) (1 + (rand () & 1));
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = (char) (1 + (rand () & 1));
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = (char) (1 + (rand () & 1));
      }
    }
  }
}

/* lemming moves */
static void
update_lemmings (void)
{
  int j;
  lemming_t *pti;
  char d, n, e, f;
  int i;

  pti = lemmings_support;
  lemmings_move_offset &= 0xffff;
  for (j = lemmings_total; j != 0; j--, pti++)
    if (pti->dead == 0) {
      d = 0;
      n = 0;
      square_lemmings_list[pti->pos1] = NULL;
      pti->pos1 = pti->pos2;
      square_lemmings_list[pti->pos1] = pti;
      e = 1;
      d = square_wall[pti->pos1];
      for (i = 0; i < 4; i++) {
	if (			/*square_wrap[(pti->pos1<<2)+i]==-1 || */
	     square_occupied[square_wrap[(pti->pos1 << 2) + i]] != 0xff ||
	     square_lemmings_list[square_wrap[(pti->pos1 << 2) + i]] != NULL)
	  d |= e;
	e += e;
      }
      f = pti->way;
      if (d & (1 << f) || f == 5) {
	e = d;
	for (i = 1; i != 16; i += i)
	  if (!(d & i))
	    n++;
	if (n != 0) {
	  n = (char) (1 + rand () % n);
	  for (i = 0; n != 0; i++, d >>= 1)
	    if (!(d & 1))
	      n--;
	  pti->way = i - 1;
	  assert ((w2d[i - 1] & e) == 0);
	  pti->pos2 = square_wrap[(pti->pos1 << 2) + pti->way];
	  assert (pti->pos2 != U32_MAX);
	} else
	  pti->way = 5;
      } else {
	pti->pos2 = square_wrap[(pti->pos1 << 2) + pti->way];
	assert (pti->pos2 != U32_MAX);
      }
      assert (pti->pos2 != U32_MAX);
      if (pti->pos1 != pti->pos2)
	square_lemmings_list[pti->pos2] = pti;

    }
}


static int
update_all (char plr)
{
  int n = 0;
  int p;
  long frames = read_htimer (update_htimer);

  for (; frames; --frames) {
    if (plr) {
      update_player (0);
      update_player (1);
      update_player (2);
      update_player (3);
      update_explo ();
      update_bonuses ();
      if (radar_current_pos < radar_target_pos)
	radar_current_pos++;
      else if (radar_current_pos > radar_target_pos)
	radar_current_pos--;
    }
    if (game_mode == M_KILLEM) {
      lemmings_move_offset += 1024;
      if (lemmings_move_offset >= 65536)
	update_lemmings ();
    }
    n++;
  }

  if (player[col2plr[0]].spec == t_tunnel && opt.inertia) {
    p = square_wrap[(player[col2plr[0]].pos << 2) + player[col2plr[0]].way];
    camera_x[0] = square_offset2coord[p << 1] << 15;
    camera_y[0] = square_offset2coord[(p << 1) + 1] << 15;
  } else {
    camera_x[0] = player[col2plr[0]].x2 << 15;
    camera_y[0] = player[col2plr[0]].y2 << 15;
  }

  if (player[col2plr[0]].way == w_left)
    camera_x[0] -= player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_up)
    camera_y[0] -= player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_right)
    camera_x[0] += player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_down)
    camera_y[0] += player[col2plr[0]].d.e >> 1;
  if (two_players) {
    if (player[col2plr[1]].spec == t_tunnel) {
      p = square_wrap[(player[col2plr[1]].pos << 2) + player[col2plr[1]].way];
      camera_x[1] = square_offset2coord[p << 1] << 15;
      camera_y[1] = square_offset2coord[(p << 1) + 1] << 15;
    } else {
      camera_x[1] = player[col2plr[1]].x2 << 15;
      camera_y[1] = player[col2plr[1]].y2 << 15;
    }
    if (player[col2plr[1]].way == w_left)
      camera_x[1] -= player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_up)
      camera_y[1] -= player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_right)
      camera_x[1] += player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_down)
      camera_y[1] += player[col2plr[1]].d.e >> 1;
  }

  return (n);
}


void
play_demo (void)
{
  int n, i;
  char notbyebye = 1;
  htimer_t demo_htimer = new_htimer (T_GLOBAL, HZ (1));
  fader_status_t fade_stat = F_UNKNOWN;

  dmsg (D_SECTION, "-- play demo --");

  in_menu = 0;
  tutor = 0;
  load_random_wrapped_level (0, 0);

  play_soundtrack ();
  set_pal_with_luminance (&tile_set_img.palette);
  radar_current_pos = 81;
  radar_target_pos = 0;
/* .... game .... */
  if (two_players) {
    nbr_tiles_cols = 8;
    camera_center_x = 436800;
  }
  inert_x[0] = camera_x[0] = player[col2plr[0]].x2 << 15;
  inert_y[0] = camera_y[0] = player[col2plr[0]].y2 << 15;
  inert_x[1] = camera_x[1] = player[col2plr[1]].x2 << 15;
  inert_y[1] = camera_y[1] = player[col2plr[1]].y2 << 15;
  init_keyboard_map ();
  n = 1;

  update_htimers ();
  reset_htimer_with_offset (event_htimer, 4);
  reset_htimer (clock_htimer);
  reset_htimer (bonus_anim_htimer);
  reset_htimer (tiles_anim_htimer);
  reset_htimer (blink_htimer);
  reset_htimer (update_htimer);
  output_screen ((char) n);	/* update corner[] */
/*   corner[0]=render_buffer[0]; */
/*   corner[1]=render_buffer[1]; */

  if (two_players == 0) {
    int flip_pos;
    pendulum_init ();
    n = 0;
    do {
      /* FIXME: use a timer for the pendulumn */
      flip_pos = pendulum_update (n);
      flip_buffer (flip_pos);
      vsynch ();
      display_buffer_tmp1 ();
      output_screen ((char) n);
      n = update_all (0);
    } while (elapsed_time < 3000);
  }
  if (two_players) {
    int buffer_pos = 39;
    do {
      vsynch ();
      display_two_buffers_moving_and_clear (buffer_pos);
      output_screen ((char) n);
      n = update_all (0);
      for (i = n; i > 0; i--)
	if (buffer_pos > 0)
	  buffer_pos--;
    } while (buffer_pos != 0);
  }

  memset (pal.global, 0, 768);
/* * * * * * * * * * *\
* MAIN LOOP in demos  *
\* * * * * * * * * * */
  dmsg (D_SECTION, "demo main loop");

  do {
    event_time = read_htimer (event_htimer);
    update_text_waving_step ();
    if (enable_blit) {
      long duration = read_htimer (demo_htimer);
      if (duration >= (DEMO_DURATION - 1) && fade_stat == F_UNKNOWN) {
	dmsg (D_MISC, "end of demo time reached");
	/* startup a black fade-out */
	std_black_fadeout (&tile_set_img.palette);
	fader_status_flagback (&fade_stat);
      }
      vsynch ();
      if (two_players == 0) {
	aff_buffer ();
      } else {
	display_two_buffers ();
      }
    }
    enable_blit = 1;
    output_screen ((char) n);
    process_input_events ();
    if (level_is_finished == 0)
      n = update_all (1);

    if (devparm && keyboard_map[HK_F12])
      level_is_finished = 1;

    if (opt.ctrl_one ^ opt.ctrl_two)
      if (is_joystick_button_b (1) && enable_blit)
	notbyebye = 0;

    if (keyboard_map[HK_Escape] && enable_blit)
      notbyebye = 0;

    if (!(notbyebye && level_is_finished == 0) && fade_stat == F_UNKNOWN) {
      /* startup a black fade-out */
      std_black_fadeout (&tile_set_img.palette);
      fader_status_flagback (&fade_stat);
    }
  } while (fade_stat != F_FINISHED);

  free_htimer (demo_htimer);
  uninit_keyboard_map ();
  unload_level ();
  nbr_tiles_cols = 15;
  camera_center_x = 873813;
  in_menu = 1;
  reset_htimer (background_htimer);
}

static void
load_demo (void)
{
  char olddeuxplr;
  char gamemodeh;
  int i;

  dmsg (D_SECTION, "-- load demo --");

  for (i = 3; i >= 0; i--)
    col2plr[i] = opt.player_color[i];
  for (i = 3; i >= 0; i--)
    plr2col[opt.player_color[i]] = i;
  gamemodeh = (rand () & 3) + 2;
  olddeuxplr = two_players;
  two_players = rand () & 1;
  if (gamemodeh == 2)
    game_mode = M_DEATHM;
  else if (gamemodeh == 3)
    game_mode = M_KILLEM;
  else if (gamemodeh == 4)
    game_mode = M_TCASH;
  else if (gamemodeh == 5)
    game_mode = M_COLOR;
  current_quest_level = 0;

  unload_level ();		/* stop also the music */
  in_demo = 1;
  if (opt.sfx)
    load_sfx_mode (game_mode);
  rounds = 1;
  play_demo ();
  game_mode = M_QUEST;
  in_demo = 0;
  two_players = olddeuxplr;
  load_random_wrapped_level (1, 0);
  load_sfx_mode (-1);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  reset_htimer (demo_trigger_htimer);
}

static void
scores_menu (void)
{
  keycode_t t;
  int i = 0, j;
  char flag = 0;
  int rolldec;
  signed char rollflag = 0;
  char points[32];

  std_white_fadein (&tile_set_img.palette);
  do {
    if (flag) {
      if (rollflag)
	event_sfx (19);
    } else
      flag = 1;
    do {
      background_menu ();
      draw_glenz_box (corner[0] + xbuf * 70, 2, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 83, 3, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 96, 4, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 109, 5, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 122, 0, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 135, 0, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 148, 0, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 161, 0, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 174, 0, 320, 6);
      draw_glenz_box (corner[0] + xbuf * 187, 0, 320, 6);
      draw_text_waving (txti[10], 159, 10, 1);
      if (!rollflag) {
	draw_text (mode_name[i], 159, 40, 1);
	for (j = 0; j < 10; j++) {
	  draw_text (highs[i][j].name, 3, 68 + 13 * j, 0);
	  sprintf (points, "%u", highs[i][j].points);
	  draw_text (points, 316, 68 + 13 * j, 2);
	}
      } else {
	draw_text_clipped_left (mode_name[i - 1], 159 - rolldec, 40, 1);
	for (j = 0; j < 10; j++) {
	  draw_text_clipped_left (highs[i - 1][j].name, 3 - rolldec,
				  68 + 13 * j, 0);
	  sprintf (points, "%u", highs[i - 1][j].points);
	  draw_text_clipped_left (points, 316 - rolldec, 68 + 13 * j, 2);
	}
	draw_text_clipped_right (mode_name[i], 159 + 320 - rolldec, 40, 1);
	for (j = 0; j < 10; j++) {
	  draw_text_clipped_right (highs[i][j].name, 3 + 320 - rolldec,
				   68 + 13 * j, 0);
	  sprintf (points, "%u", highs[i][j].points);
	  draw_text_clipped_right (points, 316 + 320 - rolldec, 68 + 13 * j, 2);
	}

      }
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (28) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (59) * xbuf + 100, 120, 3);
      vsynch ();
      aff_buffer ();
      if (rollflag == 1) {
	rolldec += 1 + (320 - rolldec) / 8;
	if (rolldec == 320)
	  rollflag = 0;
      } else if (rollflag == -1) {
	rolldec -= 1 + rolldec / 8;
	if (rolldec == 0) {
	  rollflag = 0;
	  i--;
	}
      }
    } while (!key_or_joy_ready ());
    t = get_key_or_joy ();
    if ((t == HK_Right || t == 0x3920 || t == 0x1C0D || t == HK_Down)
	&& i < 4) {
      rollflag = 1;
      rolldec = 0;
      i++;
    } else if ((t == HK_Left || t == HK_Up) && i != 0
	       && (i > 1 || rollflag != -1)) {
      if (rollflag == -1)
	i--;
      else
	rollflag = -1;
      rolldec = 320;
    }
  } while (t != HK_Escape);
  event_sfx (8);
}

static void
main_menu (void)
{
  static char l = 0;
  keycode_t t;
  char flag = 0;

  dmsg (D_SECTION, "-- menu --");

  load_random_wrapped_level (1, 0);
  load_sfx_mode (-1);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  reset_htimer (background_htimer);
  reset_htimer (demo_trigger_htimer);
  reset_htimer_with_offset (event_htimer, 4);
  event_time = read_htimer (event_htimer);
  do {
    std_white_fadein (&tile_set_img.palette);
    do {
      background_menu ();
      draw_main_menu (l);
      vsynch ();
      aff_buffer ();
      if (key_or_joy_ready () || demo_ready) {
	if (demo_ready == 0)
	  t = get_key_or_joy ();
	else
	  t = 0;
	if (t == HK_Up || t == HK_Down || t == HK_Escape)
	  event_sfx (1);
	if (t == HK_Up) {
	  if (l > 0)
	    l--;
	  else
	    l = 6;
	}
	if (t == HK_Down) {
	  if (l < 6)
	    l++;
	  else
	    l = 0;
	}
	if (t == HK_Escape) {
	  if (l != 6)
	    l = 6;
	  else
	    t = HK_Enter;
	}
	if (devparm && (t == HK_s || t == HK_S))
	  end_scroll ();
	if (t == HK_d || t == HK_D || demo_ready) {
	  htimer_t pixelize_timer;
	  long pixelize_pos;

	  event_sfx (130);
	  reset_htimer (corner_htimer);
	  for (;;) {
	    background_menu ();
	    draw_main_menu (l);
	    if (corner_buffer ())
	      break;
	    vsynch ();
	    aff_buffer ();
	  };
	  load_demo ();
	  dmsg (D_SECTION, "-- (back to) menu (from demo) --");
	  demo_ready = 0;
	  event_sfx (131);
	  std_white_fadein (&tile_set_img.palette);
	  pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
	  do {
	    background_menu ();
	    draw_main_menu (l);
	    vsynch ();
	    pixelize_pos = read_htimer (pixelize_timer);
	    if (pixelize_pos <= 6)
	      pixelize[6 - pixelize_pos] (screen, corner[0]);
	    else
	      aff_buffer ();
	  } while (pixelize_pos < 6);
	  free_htimer (pixelize_timer);
	}

      } else
	t = 0;
    } while (t != HK_Enter);
    event_sfx (10 + l);

    if (l == 0)
      play_menu ();
    if (l == 1) {
      option_menu ();
      write_options ();
    }
    if (l == 2) {
      graphic_reader ();
      event_sfx (8);
    }
    if (l == 3)
      jukebox_menu ();
    if (l == 4)
      scores_menu ();
    if (l == 5) {
      if (mouse_found)
	editor_first_menu ();
    }
    if (l == 6)
      flag = quit_menu ();
  } while (flag == 0);


  /* pixelize and fade-out, before exit */
  {
    htimer_t pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
    long pixelize_pos;
    fader_status_t fade_stat;

    std_black_fadeout (&tile_set_img.palette);
    fader_status_flagback (&fade_stat);
    do {

      background_menu ();
      draw_quit_menu (0);
      vsynch ();
      pixelize_pos = read_htimer (pixelize_timer);
      if (pixelize_pos > 6)
	pixelize_pos = 6;
      pixelize[pixelize_pos] (screen, corner[0]);
    } while (fade_stat != F_FINISHED);
    free_htimer (pixelize_timer);
  }
}

static void
pause_menu (void)
{
  int i;
  char l = 0;
  keycode_t k;
  int t, t2, dp;
  unsigned char *src = render_buffer[0];
  htimer_t pause_htimer;

  dmsg (D_SECTION, "pause menu");

  pause_htimer = new_htimer (T_GLOBAL, 1);

  halve_volume ();
  event_sfx (58);
  fastmem4 ((char *) screen, src, 64000 / 4);
  corner[0] = src;
  for (i = 64000; i != 0; i--)
    *src++ = glenz[1][*src];
  vsynch ();
  fastmem4 ((char *) corner[0], (char *) screen, 64000 / 4);
  fastmem4 ((char *) corner[0] + 90 * 320, (char *) corner[0], 20 * 320 / 4);
  fastmem4 ((char *) corner[0], (char *) corner[0] + 20 * 320, 20 * 320 / 4);
  uninit_keyboard_map ();
  do {
    update_text_waving_step ();
    fastmem4 ((char *) corner[0] + 20 * 320, (char *) corner[0],
	      20 * 320 / 4);
    draw_text_waving_320 ("PAUSE", 159, 5, 1);

    dp = read_htimer (sound_track_htimer);
    t = dp/2;
    dp &= 1;
    if (t > 5999)
      t = 5999;
    copy_rect_transp_320 (jukebox_img.buffer, corner[0] + 180 * 320 + 8, 306,
			  19);
    if (l == 0)
      copy_rect_4_320 (jukebox_img.buffer + 19 * 320,
		       corner[0] + 184 * 320 + 8 + 5, 12, 9);
    else if (l == 1)
      copy_rect_4_320 (jukebox_img.buffer + 19 * 320 + 12,
		       corner[0] + 184 * 320 + 8 + 27, 12, 9);
    else if (l == 2)
      copy_rect_4_320 (jukebox_img.buffer + 19 * 320 + 24,
		       corner[0] + 184 * 320 + 8 + 274, 16, 9);

    if (soundtrack_title)
      draw_deck_text (soundtrack_title, 110, 186, 1);
    if (soundtrack_author)
      draw_deck_text (soundtrack_author, 197, 186, 1);

    t2 = t % 60;
    t /= 60;
    copy_rect_2_320 (jukebox_img.buffer + 19 * 320 + 227 + (t2 % 10) * 6,
		     corner[0] + 186 * 320 + 8 + 228 + 19, 6, 5);
    copy_rect_2_320 (jukebox_img.buffer + 19 * 320 + 227 + (t2 / 10) * 6,
		     corner[0] + 186 * 320 + 8 + 228 + 13, 6, 5);
    if (dp == 0)
      copy_rect_2_320 (jukebox_img.buffer + 19 * 320 + 227 + 60 - 1,
		       corner[0] + 186 * 320 + 8 + 228 + 10, 2, 5);
    copy_rect_2_320 (jukebox_img.buffer + 19 * 320 + 227 + (t % 10) * 6,
		     corner[0] + 186 * 320 + 8 + 227 + 6, 6, 5);
    copy_rect_2_320 (jukebox_img.buffer + 19 * 320 + 227 + (t / 10) * 6,
		     corner[0] + 186 * 320 + 8 + 227, 6, 5);

    vsynch ();
    fastmem4 ((char *) corner[0], (char *) screen + 90 * 320, 20 * 320 / 4);
    fastmem4 ((char *) corner[0] + 180 * 320, (char *) screen + 180 * 320,
	      20 * 320 / 4);
    if (key_or_joy_ready ()) {
      k = get_key_or_joy ();
      if (k == HK_Up || k == HK_Down || k == HK_Left || k == HK_Right)
	event_sfx (140);
      if (k == HK_Up || k == HK_Left) {
	if (l > 0)
	  l--;
	else
	  l = 2;
      }
      if (k == HK_Down || k == HK_Right) {
	if (l < 2)
	  l++;
	else
	  l = 0;
      }
      if (k == HK_Enter) {
	if (l == 2)
	  k = HK_Escape;
	else {
	  if (l == 0) {
	    event_sfx (74);
	    soundtrack_current_nbr++;
	    if (soundtrack_current_nbr == 11)
	      soundtrack_current_nbr = 0;
	  }
	  if (l == 1) {
	    event_sfx (75);
	    soundtrack_current_nbr--;
	    if (soundtrack_current_nbr == -1)
	      soundtrack_current_nbr = 10;
	  }
	  unload_soundtrack ();
	  load_soundtrack_from_alias (soundtrack_list
				      [soundtrack_current_nbr]);
	  play_soundtrack ();
	  reset_htimer (sound_track_htimer);
	}
      }
      if (k == HK_Pause)
	k = HK_Escape;
    }
  } while (k != HK_Escape);

  init_keyboard_map ();
  set_volume ();
  enable_blit = 0;
  event_sfx (59);

  /* delay important timers that continued running during the pause */
  shift_htimer (update_htimer, pause_htimer);
  shift_htimer (event_htimer, pause_htimer);

  free_htimer (pause_htimer);
  dmsg (D_SECTION, "exit pause menu");
}



static char
quit_yes_no (void)
{
  int i, j;
  char l = 0;
  unsigned char *src = render_buffer[1];
  keycode_t t;
  htimer_t pause_htimer;

  dmsg (D_SECTION, "quit y/n menu");

  pause_htimer = new_htimer (T_GLOBAL, 1);

  fastmem4 ((char *) screen, src, 64000 / 4);
  for (i = 64000; i != 0; i--)
    *src++ = glenz[1][*src];
  corner[0] = render_buffer[0];

  if (opt.ctrl_one || opt.ctrl_two)
    do {
      get_joystick_state ();
      vsynch ();
    } while (joystick_b[0] || joystick_b[1]);

  uninit_keyboard_map ();
  halve_volume ();
  event_sfx (85);
  do {
    update_text_waving_step ();
    fastmem4 ((char *) render_buffer[1], corner[0], 64000 / 4);
    draw_text_waving_320 (txti[40], 159, 75, 1);
    draw_text_array_320[l == 0] (txti[41], 159, 95, 1);
    draw_text_array_320[l == 1] (txti[42], 159, 110, 1);
    j = minisinus[read_htimer (waving_htimer) & 31];
    copy_rect_transp_320 (main_font_img.buffer + 134 + 50 * 320,
			  corner[0] + j + (91 + 15 * l) * 320 + 90, 13, 20);
    copy_rect_transp_320 (main_font_img.buffer + 121 + 50 * 320,
			  corner[0] - j + (91 + 15 * l) * 320 + 320 - 90 - 13,
			  13, 20);
    vsynch ();
    fastmem4 ((char *) corner[0], (char *) screen, 64000 / 4);
    t = 0;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Down) {
	if (l != 1)
	  event_sfx (86);
	l = 1;
      }
      if (t == HK_Up) {
	if (l != 0)
	  event_sfx (86);
	l = 0;
      }
    }
  } while (t != HK_Enter);
  set_volume ();
  enable_blit = 0;
  if (l == 1)
    event_sfx (88);
  else
    event_sfx (87);
  init_keyboard_map ();

  /* delay important timers that continued running during the pause */
  shift_htimer (update_htimer, pause_htimer);
  shift_htimer (event_htimer, pause_htimer);

  reset_htimer (background_htimer);
  free_htimer (pause_htimer);
  dmsg (D_SECTION, "exit quit menu");
  return (1 - l);
}

static void
get_input_directions (void)
{				/* for humans only */
  unsigned char j = (opt.ctrl_one == 1);

  char flag1 = 0;
  char flag2 = 0;

  if (keyboard_map[HK_Pause] && enable_blit)
    pause_menu ();

  if (opt.ctrl_one == 0) {

    if (keyboard_map[opt.player_keys[0][0]] == 1) {
      player[col2plr[0]].next_way = w_up;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][1]] == 1) {
      player[col2plr[0]].next_way = w_left;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][2]] == 1) {
      player[col2plr[0]].next_way = w_down;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][3]] == 1) {
      player[col2plr[0]].next_way = w_right;
      flag1 = 1;
    }
    player[col2plr[0]].turbo =
      ((keyboard_map[opt.player_keys[0][4]] == 1) ? 2 : 1);
    if (keyboard_map[opt.player_keys[0][5]] == 1) {
      if (player[col2plr[0]].turbo == 2) {
	/* two buttons pushed */
	player[col2plr[0]].turbo = 1;
      } else
	player[col2plr[0]].turbo = 0;
    }
  } else {
    joystick_x[0] = 0;
    joystick_y[0] = 0;
    joystick_x[1] = 0;
    joystick_y[1] = 0;
    get_joystick_state ();
    if (is_joystick_up (0)) {
      player[col2plr[0]].next_way = w_up;
      flag1 = 1;
    }
    if (is_joystick_left (0)) {
      player[col2plr[0]].next_way = w_left;
      flag1 = 1;
    }
    if (is_joystick_down (0)) {
      player[col2plr[0]].next_way = w_down;
      flag1 = 1;
    }
    if (is_joystick_right (0)) {
      player[col2plr[0]].next_way = w_right;
      flag1 = 1;
    }
    player[col2plr[0]].turbo = (is_joystick_button_a (0) ? 2 : 1);
    if (is_joystick_button_b (0)) {
      if (player[col2plr[0]].turbo == 2) {
	/* two buttons pushed */
	player[col2plr[0]].turbo = 1;
      } else
	player[col2plr[0]].turbo = 0;
    }
  }

  if (player[col2plr[0]].inversed_controls != 0) {
    if (flag1)
      player[col2plr[0]].next_way ^= 2;
    player[col2plr[0]].tunnel_inverse = 1;
  } else
    player[col2plr[0]].tunnel_inverse = 0;
  if (((player[col2plr[0]].next_way ^ 2) == player[col2plr[0]].tunnel_way)
      && (player[col2plr[0]].spec == t_tunnel))
    player[col2plr[0]].next_way = player[col2plr[0]].tunnel_way;
  if (two_players == 1) {
    if (opt.ctrl_two == 0) {
      if (keyboard_map[opt.player_keys[1][0]] == 1) {
	player[col2plr[1]].next_way = w_up;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][1]] == 1) {
	player[col2plr[1]].next_way = w_left;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][2]] == 1) {
	player[col2plr[1]].next_way = w_down;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][3]] == 1) {
	player[col2plr[1]].next_way = w_right;
	flag2 = 1;
      }
      player[col2plr[1]].turbo =
	((keyboard_map[opt.player_keys[1][4]] == 1) ? 2 : 1);
      if (keyboard_map[opt.player_keys[1][5]] == 1) {
	if (player[col2plr[1]].turbo == 2) {
	  /* two buttons pushed */
	  player[col2plr[1]].turbo = 1;
	} else
	  player[col2plr[1]].turbo = 0;
      }
    } else {
      if (!j) {
	joystick_x[0] = 0;
	joystick_y[0] = 0;
	get_joystick_state ();
      }
      if (is_joystick_up (j)) {
	player[col2plr[1]].next_way = w_up;
	flag2 = 1;
      }
      if (is_joystick_left (j)) {
	player[col2plr[1]].next_way = w_left;
	flag2 = 1;
      }
      if (is_joystick_down (j)) {
	player[col2plr[1]].next_way = w_down;
	flag2 = 1;
      }
      if (is_joystick_right (j)) {
	player[col2plr[1]].next_way = w_right;
	flag2 = 1;
      }
      player[col2plr[1]].turbo = (is_joystick_button_a (j) ? 2 : 1);
      if (is_joystick_button_b (j)) {
	if (player[col2plr[1]].turbo == 2) {
	  /* two buttons pushed */
	  player[col2plr[1]].turbo = 1;
	} else
	  player[col2plr[1]].turbo = 0;
      }
    }
    if (player[col2plr[1]].inversed_controls != 0) {
      if (flag2)
	player[col2plr[1]].next_way ^= 2;
      player[col2plr[1]].tunnel_inverse = 1;
    } else
      player[col2plr[1]].tunnel_inverse = 0;

/*   if ((player[col2plr[1]].spec==t_tunnel)) printf("ss:%d,nxss:%d,ssold:%d,ssold2:%d,sstun:%d.\n", */
/*       player[col2plr[1]].way,player[col2plr[1]].next_way,player[col2plr[1]].old_way,player[col2plr[1]].old_old_way,player[col2plr[1]].tunnel_way); */
    if (((player[col2plr[1]].next_way ^ 2) == player[col2plr[1]].tunnel_way)
	&& (player[col2plr[1]].spec == t_tunnel))
      player[col2plr[1]].next_way = player[col2plr[1]].tunnel_way;
  }
  if (opt.ctrl_one ^ opt.ctrl_two) {
    if (is_joystick_button_a (1) && enable_blit)
      keyboard_map[HK_Pause] = 1;
  }
  if (keyboard_map[HK_PrintScreen] && snap)
    save_pcx (0);
  if (keyboard_map[HK_SysRq] && snap)
    save_pcx (1);
}

static void
draw_end_level_info (int decal, char l)
{
  int i, j;
  char winner[128];
  char nbr[32];

  if (level_is_finished != 15) {
    sprintf (winner, txti[50], plr2col[level_is_finished - 1] + 1);
    draw_glenz_box (corner[0] + decal + 22 * xbuf, level_is_finished + 1, 320,
		    6);
  } else {
    if (two_players)
      sprintf (winner, txti[51]);
    else
      sprintf (winner, txti[52]);
    draw_glenz_box (corner[0] + decal + 22 * xbuf, 7, 320, 6);
  }

  draw_text_waving (winner, 159 + decal, 20, 1);
  if (game_mode == M_QUEST)
    draw_text (txti[53], 180 + decal, 50, 1);
  else if (game_mode == M_DEATHM)
    draw_text (txti[54], 221 + decal, 50, 1);
  else if (game_mode == M_KILLEM)
    draw_text (txti[55], 180 + decal, 50, 1);
  else if (game_mode == M_TCASH)
    draw_text (txti[56], 180 + decal, 50, 1);
  else if (game_mode == M_COLOR)
    draw_text (txti[57], 170 + decal, 50, 1);
  if ((level_is_finished != 15) && (game_mode == M_QUEST)) {
    j = minisinus[read_htimer (waving_htimer) & 31];
    draw_text_array[l == 0] (txti[58], 159 + decal, 150, 1);
    draw_text_array[l == 1] (txti[59], 159 + decal, 170, 1);

    exec_rleprog (left_arrow,
		  corner[0] + decal + (145 + l * 20) * xbuf + 45 + j);
    exec_rleprog (right_arrow,
		  corner[0] + decal + (145 + l * 20) * xbuf
		  + 320 - 45 - 13 - j);
  } else {
    draw_text (txti[60], 159 + decal, 160, 1);
  }
  for (i = 0; i < 4; i++) {
    draw_glenz_box (corner[0] + decal + (75 + i * 12) * xbuf +
		    2 * xbuf /*+25+28 */ , col2plr[i] + 2,
		    284 /*-25-28*/  + i * 6, 6);
    copy_rect_transp (vehicles_img.buffer + 16 + 64 * col2plr[i],
		      corner[0] + decal + (75 + i * 12) * xbuf + 284 + i * 6,
		      12, 10);
    copy_rect_4 (main_font_img.buffer + 196 + col2plr[i] * 28 + 72 * 320,
		 corner[0] + decal + (75 + i * 12) * xbuf + /*25 */ 5, 28,
		 11);
    if (player[col2plr[i]].martians_nbr)
      copy_rect_transp_shadow (main_font_img.buffer + 120 * 320 + 40 + i * 64,
			       corner[0] + decal + (69 + i * 12) * xbuf + 35,
			       24, 19);
    if (game_mode == M_QUEST)
      sprintf (nbr, "%d", (trail_size[col2plr[i]] + 1) / 5 - 1);
    else if (game_mode == M_DEATHM)
      sprintf (nbr, "   ");
    else if (game_mode == M_KILLEM)
      sprintf (nbr, "%d", player[col2plr[i]].lemmings_nbr);
    else if (game_mode >= M_TCASH)
      sprintf (nbr, "%d", player[col2plr[i]].cash);
    draw_text (nbr, 108 - 10 + decal, 75 + i * 12, 1);
    sprintf (nbr, "%d", player[col2plr[i]].score);
    draw_text (nbr, 182 - 10 + decal, 75 + i * 12, 1);
    sprintf (nbr, "%d", player[col2plr[i]].lifes);
    draw_text (nbr, 265 - 10 + decal, 75 + i * 12, 1);
  }
}

static void
draw_round_info (int decal)
{
  int i;
  char info[128];

  sprintf (info, txti[61], rounds_nbr_values[opt.gamerounds] - rounds + 1,
	   rounds_nbr_values[opt.gamerounds]);
  draw_glenz_box (corner[0] + decal + 22 * xbuf, 1, 320, 6);
  draw_text_waving (info, 159 + decal, 20, 1);
  draw_text (txti[62], 180 + decal, 50, 1);

  draw_text (txti[60], 159 + decal, 160, 1);

  for (i = 0; i < 4; i++) {
    draw_glenz_box (corner[0] + decal + (75 + i * 12) * xbuf +
		    2 * xbuf /*+25+28 */ , col2plr[i] + 2,
		    284 /*-25-28*/  + i * 6, 6);
    copy_rect_transp (vehicles_img.buffer + 16 + 64 * col2plr[i],
		      corner[0] + decal + (75 + i * 12) * xbuf + 284 + i * 6,
		      12, 10);
    copy_rect_4 (main_font_img.buffer + 196 + col2plr[i] * 28 + 72 * 320,
		 corner[0] + decal + (75 + i * 12) * xbuf + /*25 */ 5, 28,
		 11);
    sprintf (info, "%d", player[col2plr[i]].wins);
    draw_text (info, 108 - 10 + decal, 75 + i * 12, 1);
    sprintf (info, "%d", player[col2plr[i]].score);
    draw_text (info, 182 - 10 + decal, 75 + i * 12, 1);
    sprintf (info, "%d", player[col2plr[i]].lifes);
    draw_text (info, 265 - 10 + decal, 75 + i * 12, 1);
  }
}




static unsigned char
play_game (char cont)
{
  int n, i, t;
  char notbyebye = 1, flag;
  int l, pos, u;
  char editflag = 0;
  static char tmpname[20];
  char bufstr[32];

  dmsg (D_SECTION, "-- play game --");

  in_menu = 0;
  tutor = (char) (game_mode == M_QUEST && current_quest_level == 0);
  if (loadulevel == 1) {
    char* tmp = get_non_null_rsc_file ("levels-dir");
    strappend (tmp, level_name);
    if (load_level (level_name, cont))
      emsg ("Error during level loading");
    free (tmp);
  } else if (game_mode == M_QUEST /*&& questmode==0 */ )
    load_level_from_number (current_quest_level++, cont);
/* else if (game_mode==M_QUEST) loadlvlpasrandq2(current_quest_level++,cont); */
  else
    load_random_level (cont);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  set_pal_with_luminance (&tile_set_img.palette);
  radar_current_pos = 81;
  radar_target_pos = 0;

/* . ... game ... . */
  if (two_players) {
    nbr_tiles_cols = 8;
    camera_center_x = 436800;
  }
  inert_x[0] = camera_x[0] = player[col2plr[0]].x2 << 15;
  inert_y[0] = camera_y[0] = player[col2plr[0]].y2 << 15;
  inert_x[1] = camera_x[1] = player[col2plr[1]].x2 << 15;
  inert_y[1] = camera_y[1] = player[col2plr[1]].y2 << 15;
  init_keyboard_map ();
  n = 1;

  update_htimers ();
  reset_htimer_with_offset (event_htimer, 4);
  reset_htimer (clock_htimer);
  reset_htimer (bonus_anim_htimer);
  reset_htimer (tiles_anim_htimer);
  reset_htimer (blink_htimer);
  reset_htimer (update_htimer);

  dmsg (D_SECTION, "introduce game");

  output_screen ((char) n);	/* update corner[] */
  process_input_events ();

  if (game_mode == M_QUEST)
    sprintf (bufstr, txti[65], current_quest_level);
  else
    sprintf (bufstr, txti[66], rounds_nbr_values[opt.gamerounds] - rounds + 1,
	     rounds_nbr_values[opt.gamerounds]);

  if (two_players == 0) {
    int pendulum_pos;
    pendulum_init ();
    n = 0;
    do {
      pixel_t* tmp;
      pendulum_pos = pendulum_update (n);
      flip_buffer (pendulum_pos);

      tmp = corner[0];
      corner[0] = render_buffer[1];
      draw_text (bufstr, 159, 99, 1);
      corner[0] = tmp;

      vsynch ();
      display_buffer_tmp1 ();
      output_screen ((char) n);
      process_input_events ();
      n = update_all (0);
    } while (elapsed_time < 3000);
  }
  if (two_players) {
    pixel_t *tmp;
    int buffer_pos = 39;

    memset (screen, 0, 64000);
    tmp = corner[0];
    corner[0] = screen;
    draw_text_320 (bufstr, 159, 99, 1);
    corner[0] = tmp;

    sleep (1);

    n = 1;
    do {
      corner[0] = corner[swapside];
      draw_text (bufstr, 159 + (buffer_pos << 2), 99, 1);
      corner[0] = tmp;
      corner[0] = corner[1 - swapside];
      draw_text (bufstr, 159 - (buffer_pos << 2) - 160, 99, 1);
      corner[0] = tmp;
      vsynch ();
      display_two_buffers_moving (buffer_pos);
      output_screen ((char) n);
      process_input_events ();
      n = update_all (0);
      for (i = n; i > 0; i--)
	if (buffer_pos > 0)
	  buffer_pos--;
    } while (buffer_pos != 0);

  }
  event_sfx (141);

/* * * * * * * * * * * * * * *\
* MAIN LOOP during the games *
\* * * * * * * * * * * * * * */

  dmsg (D_SECTION, "game main loop");

  do {
    event_time = read_htimer (event_htimer);
    update_text_waving_step ();
    if (enable_blit) {
      if (two_players == 0) {
	vsynch ();
	aff_buffer ();
      } else {
	vsynch ();
	display_two_buffers ();
      }
    }
    enable_blit = 1;
    output_screen ((char) n);
    process_input_events ();
    n = update_all (1);
    get_input_directions ();
    if (devparm && keyboard_map[HK_F12])
      level_is_finished = 1;
    if (opt.ctrl_one ^ opt.ctrl_two)
      if (is_joystick_button_b (1) && enable_blit)
	notbyebye = quit_yes_no ();
    if (keyboard_map[HK_Escape] && enable_blit)
      notbyebye = quit_yes_no ();
  } while (notbyebye && level_is_finished == 0);

  dmsg (D_SECTION, "game finished");

  if (level_is_finished >= 1 && level_is_finished <= 4)
    player[level_is_finished - 1].wins++;

  radar_target_pos = 81;
  if (notbyebye) {

    dmsg (D_SECTION, "print end level info");

    uninit_keyboard_map ();
    l = 0;
    if (level_is_finished != 15)
      if (player[level_is_finished - 1].cpu == 2)
	event_sfx (63);
      else
	event_sfx (64);
    else
      event_sfx (65);
    do {
      if (two_players == 0) {
	draw_end_level_info (0, l);
	vsynch ();
	aff_buffer ();
      } else {
	pixel_t* tmp;

	tmp = corner[0];
	draw_end_level_info (swapside ? -160 : 0, l);
	corner[0] = corner[1];
	draw_end_level_info (swapside ? 0 : -160, l);
	corner[0] = tmp;
	vsynch ();
	display_two_buffers ();
      }
      output_screen ((char) n);
      n = update_all (1);
      t = 0;
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
	if (t == HK_Down && game_mode == M_QUEST)
	  l = 1;
	if (t == HK_Up)
	  l = 0;
      }
      flag = 1;
      if (opt.ctrl_one || opt.ctrl_two) {
	get_joystick_state ();
	if (is_joystick_down (0))
	  l = 1;
	if (is_joystick_up (0))
	  l = 0;
	if (is_joystick_button_a (0))
	  flag = 0;
	if (opt.ctrl_one && opt.ctrl_two) {
	  if (is_joystick_down (1))
	    l = 1;
	  if (is_joystick_up (1))
	    l = 0;
	  if (is_joystick_button_a (1))
	    flag = 0;
	}
      }
      if (l == 1 && (t == HK_Enter || flag == 0) && game_mode == M_QUEST) {
	event_sfx (67);
	t = 0;
	l = 0;
	editflag = 0;
	do {
	  if (two_players == 0) {
	    draw_saved_games_info (0, l, 1);
	    vsynch ();
	    aff_buffer ();
	  } else {
	    pixel_t *tmp;

	    tmp = corner[0];
	    draw_saved_games_info (swapside ? -160 : 0, l, 1);
	    corner[0] = corner[1];
	    draw_saved_games_info (swapside ? 0 : -160, l, 1);
	    corner[0] = tmp;
	    vsynch ();
	    display_two_buffers ();
	  }
	  output_screen ((char) n);
	  n = update_all (1);
	  if (key_or_joy_ready ()) {
	    if (editflag == 2)
	      editflag = 0;
	    if (editflag == 0) {
	      t = get_key_or_joy ();
	      if (t == HK_Down) {
		((l < 9) ? (l++) : (l = 0));
		event_sfx (120);
	      }
	      if (t == HK_Up) {
		((l > 0) ? (l--) : (l = 9));
		event_sfx (120);
	      }
	      if (t == 0x0e7f) {
		saverec[l].used = 0;
		saverec[l].name[0] = 0;
		event_sfx (128);
	      }
	      if (t == HK_Enter) {
		editflag = 1;
		strcpy (tmpname, saverec[l].name);
		pos = strlen (saverec[l].name);
		saverec[l].name[pos] = '^';
		saverec[l].name[pos + 1] = 0;
		event_sfx (125);
	      }
	      if (t == HK_Escape)
		event_sfx (123);
	    } else {
	      t = get_key_or_joy ();
	      u = t & 255;
	      pos = strlen (saverec[l].name);
	      if (u >= 'a' && u <= 'z')
		u -= 'a' - 'A';
	      if (pos < 13)
		if ((u > 20 && u <= 95) || (u == 20 && pos != 0)) {
		  saverec[l].name[pos - 1] = u;
		  saverec[l].name[pos] = '^';
		  saverec[l].name[pos + 1] = 0;
		  event_sfx (121);
		}
	      if ((t == HK_BackSpace || t == HK_Delete) && (pos > 1)) {
		saverec[l].name[pos - 1] = 0;
		saverec[l].name[pos - 2] = '^';
		event_sfx (122);
	      }
	      if (t == 0x1c0a) {
		saverec[l].name[pos - 1] = 0;
		event_sfx (127);
		editflag = 2;
	      }
	      if (t == HK_Escape) {
		strcpy (saverec[l].name, tmpname);
		event_sfx (123);
		editflag = 2;
	      }
	      if (t == HK_Enter) {
		saverec[l].name[pos - 1] = 0;
		saverec[l].level = current_quest_level /*+1 */ ;
/*                saverec[l].questmode=questmode; */
		for (u = 0; u < 4; u++) {
		  saverec[l].points[u] = player[col2plr[u]].score;
		  saverec[l].magic = game_magic;
		  saverec[l].lifes[u] = player[col2plr[u]].lifes;
		}
		saverec[l].used = 1;
		editflag = 0;
		event_sfx (124);
	      }
	    }
	  }
	} while ((t != HK_Escape && t != HK_Enter) || editflag != 0);
	/* while (keyboard_map[HK_Escape]) process_input_events (); */
	l = 1;
	write_save_records ();
      } else if (t == HK_Escape) {
	if (joystick_detected & 1)
	  do
	    get_joystick_state ();
	  while (is_joystick_button_a (0) != 0);
	if (quit_yes_no () == 0)
	  l = 255;
      }
    } while ( /*keyboard_map[HK_Escape]==0 */ l != 255
	     && ((t != HK_Enter && flag) || l == 1));
    init_keyboard_map ();

    if (l == 255 || game_mode == M_QUEST) {
      if (l == 0)
	event_sfx (66);		/* next level */
      if (l == 1)
	event_sfx (67);		/* save game */
      if (l == 255)
	event_sfx (68);		/* echap */
      for (i = 1; i <= 40; i += n) {
	if (two_players == 0) {
	  draw_end_level_info (0, l);
	  vsynch ();
	  display_buffer_moving (i);
	} else {
	  pixel_t *tmp;

	  tmp = corner[0];
	  draw_end_level_info (swapside ? -160 : 0, l);
	  corner[0] = corner[1];
	  draw_end_level_info (swapside ? 0 : -160, l);
	  corner[0] = tmp;
	  vsynch ();
	  display_two_buffers_moving_and_clear (i);
	}
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
      }
      if (two_players == 0) {
	draw_end_level_info (0, l);
	vsynch ();
	display_buffer_moving (40);
      } else {
	pixel_t *tmp;
	tmp = corner[0];
	draw_end_level_info (swapside ? -160 : 0, l);
	corner[0] = corner[1];
	draw_end_level_info (swapside ? 0 : -160, l);
	corner[0] = tmp;
	vsynch ();
	display_two_buffers_moving_and_clear (40);
      }
    } else {
      dmsg (D_SECTION, "print round info");

      l = 0;
      flag = 1;

      event_sfx (129);
      do {
	if (two_players == 0) {
	  draw_round_info (0);
	  vsynch ();
	  aff_buffer ();
	} else {
	  pixel_t *tmp;
	  tmp = corner[0];
	  draw_round_info (swapside ? -160 : 0);
	  corner[0] = corner[1];
	  draw_round_info (swapside ? 0 : -160);
	  corner[0] = tmp;
	  vsynch ();
	  display_two_buffers ();
	}
	if (keyboard_map[HK_Enter] == 0)
	  flag = 0;
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
	if (keyboard_map[HK_Escape] != 0)
	  if (quit_yes_no () == 0)
	    l = 255;
      } while ( /*keyboard_map[HK_Escape]==0 */ l != 255
	       && (flag || keyboard_map[HK_Enter] == 0));
/*   if (keyboard_map[HK_Escape]) l=255; */

      if (l == 0)
	event_sfx (66);		/*next */
      if (l == 1)
	event_sfx (67);		/*save */
      if (l == 255)
	event_sfx (68);		/*esc */
      for (i = 1; i <= 40; i += n) {
	if (two_players == 0) {
	  draw_round_info (0);
	  vsynch ();
	  display_buffer_moving (i);
	} else {
	  pixel_t *tmp;
	  tmp = corner[0];
	  draw_round_info (swapside ? -160 : 0);
	  corner[0] = corner[1];
	  draw_round_info (swapside ? 0 : -160);
	  corner[0] = tmp;
	  vsynch ();
	  display_two_buffers_moving_and_clear (i);
	}
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
      }
      if (two_players == 0) {
	draw_round_info (0);
	vsynch ();
	display_buffer_moving (40);
      } else {
	pixel_t *tmp;
	tmp = corner[0];
	draw_round_info (swapside ? -160 : 0);
	corner[0] = corner[1];
	draw_round_info (swapside ? 0 : -160);
	corner[0] = tmp;
	vsynch ();
	display_two_buffers_moving_and_clear (40);
      }
/* end of round info */
    }

  }
  if ((!notbyebye) || (level_is_finished == 15))
    l = 255;
  uninit_keyboard_map ();
  unload_level ();
  nbr_tiles_cols = 15;
  camera_center_x = 873813;
  in_menu = 1;
/* if (l!=0) cont=0; */

  return (l);
}

static void
read_level_list (void)
{
  FILE *f;
  int i = 0;
  char string[32];
  char* t = get_non_null_rsc_file ("levels-list-txt");

  dmsg (D_FILE|D_SECTION, "read level list: %s ...", t);
  if ((f = fopen (t, "rt")) == NULL) {
    dperror ("fopen");
    emsg ("Could not open %s.", t);
  }
  free (t);
  while (!feof (f)) {
    fgets ((char *) string, 32, f);
    if (string[0] == '>' || string[0] == ' ')
      level_list_nbr++;
  }
  fseek (f, 0, 0);
  level_list = malloc (level_list_nbr * 13 * sizeof (char));
  levelinf = malloc (level_list_nbr);
  while (!feof (f)) {
    char *tmp;
    fgets ((char *) string, 32, f);
    tmp = strchr ((char *) string, 0xd);
    if (tmp)
      *tmp = 0;
    tmp = strchr ((char *) string, 0xa);
    if (tmp)
      *tmp = 0;
    string[13] = 0;
    if (string[0] == '>' || string[0] == ' ') {
      strncpy (level_list + i * levellstchunk, (char *) &(string[1]), 13);
      if (string[0] == '>')
	levelinf[i] = 1;
      else
	levelinf[i] = 0;
      i++;
    }
  }
  fclose (f);
  dmsg (D_FILE|D_SECTION, "... done");
}

/*
static void readlvllstq2(void)
{ FILE *f;int i=0;
  char string[32];
  if ((f=fopen(nivdir"q2.lst","rt"))==NULL) fatal_error("Q2.LST not found");
  while (!feof(f))
  {
   fgets((char*)string,32,f);
   if (string[0]!=0) levelnbrq2++;
  }
  fseek(f,0,0);
  levellstq2=malloc(levelnbrq2*13);
  while (!feof(f))
  {
   fgets((char*)string,32,f);
   *strchr((char*)string,0xd)=0;
   *strchr((char*)string,0xa)=0;
   string[13]=0;
   strncpy(levellstq2[i],(char*)&(string[0]),13);
   i++;
  }
  fclose(f);
}
*/


int
main (int argc, char *argv[])
{
  int i;

  dmsg_init (argv[0]);
  dmsg (D_SECTION,"initialization");

  {
    char* data_dir;
    dmsg (D_SYSTEM,"looking for HEROES_DATA_DIR or HEROES_DATADIR...");
    if ((data_dir = getenv ("HEROES_DATA_DIR")) ||
	(data_dir = getenv ("HEROES_DATADIR"))) {
      dmsg (D_SYSTEM,"... found: %s", data_dir);
      set_rsc_file ("data-dir", data_dir);
    } else {
      dmsg (D_SYSTEM, "... not found.");
      set_rsc_file ("data-dir", datadir);
    }
  }
  {
    char* home_dir;
    dmsg (D_SYSTEM,"looking for HEROES_HOME_DIR, HEROES_HOMEDIR or HOME...");
    if ((home_dir = getenv ("HEROES_HOME_DIR")) ||
	(home_dir = getenv ("HEROES_HOMEDIR")) ||
	(home_dir = getenv ("HOME"))) {
      dmsg (D_SYSTEM,"... found: %s", home_dir);
      set_rsc_file ("home-dir", home_dir);
    } else {
      dmsg (D_SYSTEM, "... not found.");
      wmsg ("HOME variable not found in environment, defaulting to `.'");
      set_rsc_file ("home-dir", ".");
    }
  }

  init_sound_track_list ();

  /* Read the system-wide configuration file. */
  {
    char* tmp;
    tmp = get_rsc_file ("system-conf");
    if (tmp) {
      read_userconf (tmp, argv[0]);
      free (tmp);
    }
  }

  if (setup_userdir ())
    exit (1);

  add_default_extra_directories ();

  /* Read the user configuration file. */
  {
    char* tmp;
    tmp = get_rsc_file ("user-conf");
    if (tmp) {
      read_userconf (tmp, argv[0]);
      free (tmp);
    }
  }

  dmsg (D_SYSTEM, "parsing command line");
  if (parse_argv (argc, argv))
    exit (1);

  read_txti_cfg ();

  dmsg (D_SYSTEM, "randomize");
  srand (time (0));

#ifdef PORT
  /* FIXME: */
  run1st ();
#endif

  read_level_list ();
/* readlvllstq2(); */
  browse_extra_directories ();
  if (reinitopt)
    reinit_options ();
  else
    load_options ();

  if (reinitsco)
    clear_scores ();
  else
    load_scores ();

  if (reinitsav)
    clear_save_records ();
  else
    load_save_records ();

  if (read_sfx_conf ())
    emsg ("error in sfx.cfg");

  if (joyoff) {
    joystick_detected = 0;
    /* reset controlers configuration to keyboards */
    opt.ctrl_one = 0;
    opt.ctrl_two = 0;
  } else
    joyinit ();

#ifdef PORT
  /* FIXME: MOUSE */
  if (mouseinit () == -1)
    mouse_found = 1;
#endif

  if (init_sound_engine ())
    exit (2);

  init_video ();

  init_htimer ();
  clock_htimer = new_htimer (T_GLOBAL, HZ (10));
  blink_htimer = new_htimer (T_GLOBAL, HZ (6));
  event_htimer = new_htimer (T_GLOBAL, HZ (70));
  waving_htimer = new_htimer (T_GLOBAL, HZ (70));
  corner_htimer = new_htimer (T_GLOBAL, HZ (280));
  update_htimer = new_htimer (T_LOCAL, HZ (70));
  background_htimer = new_htimer (T_LOCAL, HZ (70));
  sound_track_htimer = new_htimer (T_GLOBAL, HZ (2));
  tiles_anim_htimer = new_htimer (T_GLOBAL, HZ (70));
  demo_trigger_htimer = new_htimer (T_GLOBAL, HZ (1));
  init_text_waving_step ();
  init_fader ();

  if (!directmenu) {
    play_intro ();
  }

  init_buffers ();
  init_bonuses ();

  pcx_load_from_rsc ("main-font", &main_font_img);
  pcx_load_from_rsc ("menu-pictures-img", &icons_img);
  pcx_load_from_rsc ("vehicles-img", &vehicles_img);
  pcx_load_from_rsc ("trails-img", &trailimg);
/* if (odbg) debugsavepcx(&trailimg); */
  pcx_load_from_rsc ("jukebox-img", &jukebox_img);
  pcx_load_from_rsc ("jukebox-font", &font_deck_img);

  init_fonts ();

  for (i = nfrexplo1 - 1; i >= 0; i--) {
    fst_explo_list[i] += (int) vehicles_img.buffer;
    snd_explo_list[i] += (int) vehicles_img.buffer;
  }
  for (i = 15; i >= 0; i--) {
    trail[i] += (int) trailimg.buffer;
  }
  for (i = 0; i != 32; i++)
    minisinus[i] = ceil (sin (i * 2.0 * 3.141592653 / 32.0) * 1.7);
  compute_lut ();

  init_menus_sprites ();
  update_htimers ();
  main_menu ();

  uninit_menus_sprites ();
  uninit_fader ();
  uninit_text_waving_step ();

  uninit_fonts ();

  free_htimer (demo_trigger_htimer);
  free_htimer (sound_track_htimer);
  free_htimer (tiles_anim_htimer);
  free_htimer (background_htimer);
  free_htimer (update_htimer);
  free_htimer (waving_htimer);
  free_htimer (event_htimer);
  free_htimer (corner_htimer);
  free_htimer (clock_htimer);
  free_htimer (blink_htimer);

  img_free (&font_deck_img);
  img_free (&jukebox_img);
  img_free (&trailimg);
  img_free (&vehicles_img);
  img_free (&icons_img);
  img_free (&main_font_img);
  free_all_sfx ();
  close_sfx_handle ();
  unload_level ();
  free (levelinf);
  free (level_list);
  uninit_bonuses ();
  close_buffers ();
  uninit_sound_engine ();
  uninit_video ();
  uninit_sound_track_list ();
  free_extra_list ();
  free_extra_directories ();
  write_save_records ();
  write_scores ();
  write_options ();
  free_save_records ();
  free_scores ();
  free_options ();
  free_userdir ();
  close_txti ();
  return 0;
}
