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

#include "system.h"
#include "display.h"
#include "pcx.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "joystick.h"
#include "errors.h"
#include "fastmem.h"
#include "sfx.h"
#include "prefs.h"
#include "scores.h"
#include "savegame.h"
#include "intro.h"
#include "menus.h"
#include "extras.h"
#include "visuals.h"
#include "render.h"
#include "renderdata.h"
#include "pixelize.h"
#include "misc.h"
#include "argv.h"
#include "debugmsg.h"

#include "sound.h"
#include "endscroll.h"

#include "heroes.h"
#include "userdir.h"
#include "userconf.h"
#include "musicfiles.h"
#include "bytesex.h"
#include "structs.h"
#include "hendian.h"
#include "rsc_files.h"
#include "rsc_files_hash.h"
#include "fader.h"
#include "const.h"
#include "scrtools.h"
#include "fontdata.h"
#include "bonus.h"
#include "sprtext.h"
#include "explosions.h"
#include "items.h"
#include "sprprogwav.h"
#include "gameid.h"
#include "persona.h"
#include "relocate.h"
#include "vars.h"

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

static unsigned char play_game (char);

static void
init_buffers (void)
{
  XMALLOC_ARRAY (render_buffer[0], xbuf * ybuf);
  XMALLOC_ARRAY (render_buffer[1], xbuf * ybuf);
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

/* Each tunnel has two input/output squares, indiced 0 and 1.
   tunnel_square_io[][] is used to build the map of square links as it
   helps to locate the square used to exit from a tunnel, given the
   direction of the output tunnel (first index), and the  square
   input number (second index).
   For instance, if tile A is a tunnel oriented up, linked to
   tile B which is a tunnel oriented right, we need to:
    link  A's square number 'tunnel_square_io[w_up][0]'
      to  B's square number 'tunnel_square_io[w_right][0]'
   and link  A's square number 'tunnel_square_io[w_up][1]'
         to  B's square number 'tunnel_square_io[w_right][1]'
*/
int tunnel_square_io[4][2] = { {0, 1}, {1, 3}, {3, 2}, {2, 0} };

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

  XMALLOC_ARRAY (level_map, map_info.xt * map_info.yt);

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
  } else {
    load_soundtrack_from_alias (map_info.soundtrack_name);
  }

  dmsg (D_LEVEL, "initialize variables and maps associated to the level");

  map_info_2xt = map_info.xt << 1;
  map_info_2yt = map_info.yt << 1;
  map_info_2xwrap = (map_info.xwrap << 1) + 1;
  map_info_2ywrap = (map_info.ywrap << 1) + 1;

  XSALLOC_ARRAY (square_occupied, map_info_2xt * map_info_2yt, 0xff);
  XCALLOC_ARRAY (square_radar_wall, map_info_2xt * map_info_2yt);
  XCALLOC_ARRAY (square_wall, map_info_2xt * map_info_2yt);
				/* What are the following `+ 1' for? */
  XSALLOC_ARRAY (square_explosion, map_info_2xt * map_info_2yt + 1, 254);
  XCALLOC_ARRAY (square_dead_explosion, map_info_2xt * map_info_2yt + 1);
  last_explo = 0;

  XMALLOC_ARRAY (square_explosion_type, map_info_2xt * map_info_2yt + 1);
  for (i = map_info_2xt * map_info_2yt - 1; i >= 0; i--)
    square_explosion_type[i] = rand () % NBR_EXPLOSION_KINDS;

  XMALLOC_ARRAY (square_way, map_info_2xt * map_info_2yt);

  if (init_bonuses_level ())
    return 15;

  XMALLOC_ARRAY (square2tile, map_info_2xt * map_info_2yt);
  k = 0;
  for (i = 0, l = 0; i < (int)map_info.yt; i++, l += map_info_2xt * 2) {
    for (j = 0, k2 = l; j < (int)map_info.xt; j++, k2 += 2, k++) {
      square2tile[k2] = k;
      square2tile[k2 + 1] = k;
      square2tile[map_info_2xt + k2] = k;
      square2tile[map_info_2xt + k2 + 1] = k;
    }
  }

  XCALLOC_ARRAY (square_wrap, map_info_2xt * map_info_2yt * 4);
  XCALLOC_ARRAY (square_offset2coord, map_info_2xt * map_info_2yt * 2);

  if (game_mode == M_KILLEM) {
    XCALLOC_ARRAY (square_lemmings_list, map_info_2xt * map_info_2yt);
    XCALLOC_ARRAY (square_dead_lemmings_list, map_info_2xt * map_info_2yt);
    memset (lemmings_support, 0, lemmings_total * sizeof (lemming_t));
  }
  if (game_mode >= M_TCASH) {
    XMALLOC_ARRAY (square_object, map_info_2xt * map_info_2yt);
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
    XMALLOC_ARRAY (explo_list_ptr, explo_nbr);
    XMALLOC_ARRAY (explo_list_pos_x, explo_nbr);
    XMALLOC_ARRAY (explo_list_pos_y, explo_nbr);
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
    /* when the framerate is too low, don't do inerta */
    if (n > 16)
      n = 16;

    if (map_info.xwrap == DONT_WRAP)
      inert_x[p] = camera_x[p] =
	inert_x[p] + n * (camera_x[p] - inert_x[p]) / 16;
    else {
      d1 = camera_x[p] - inert_x[p];
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
	inert_y[p] + n * (camera_y[p] - inert_y[p]) / 16;
    else {
      d1 = camera_y[p] - inert_y[p];
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
    emsg (_("Error %d during loading level"), e);
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
  XMALLOC_ARRAY (level_full_list, level_full_list_size);

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
    emsg (_("Error %d occured while loading level %s"), e, tmp);
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
    emsg (_("Error %d occured while loading level %s"), e, tmp);
  }

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
      flush_display (corner[0]);
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
      load_save_records ();
      std_white_fadein (&tile_set_img.palette);
      do {
	background_menu ();
	draw_saved_games_info (0, u, false);
	flush_display (corner[0]);
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
  } while (l == 0 || (l == 6 && flagload == 0));
  event_sfx (9);

  flip_timer = new_htimer (T_GLOBAL, HZ (280));
  do {
    if (flagload == 0)
      draw_play_menu (l);
    else {
      background_menu ();
      draw_saved_games_info (0, u, false);
    }
    flip_pos = -read_htimer (flip_timer);
    if (flip_pos < -256)
      flip_pos = -256;
    flip_buffer (flip_pos);
    flush_display (render_buffer[1]);
  } while (flip_pos > -256);
  flush_display (render_buffer[1]);
  /* erase the one-line flipped screen */
  memset (render_buffer[1] + 100 * xbuf, 0, 320);
  flush_display (render_buffer[1]);

  free_htimer (flip_timer);

  if (flagload == 0) {
    if (l == 1 )
      game_mode = M_QUEST;
    else if (l == 2)
      game_mode = M_DEATHM;
    else if (l == 3)
      game_mode = M_KILLEM;
    else if (l == 4)
      game_mode = M_TCASH;
    else if (l == 5)
      game_mode = M_COLOR;
    gamemodeh = game_mode;
    cont = 0;
    current_quest_level = 0;
    create_gameid (game_id);
  } else {
    gamemodeh = game_mode = M_QUEST;
    current_quest_level = saverec[u].level;
    copy_gameid (game_id, saverec[u].gid);
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
  /* FIXME: Ideally, we should only lock the file if there
     are a score update planned.  And not when the user
     is interacting.  */
  load_scores_and_keep_locked ();
  for (t = 0; t < 4; t++)
    if (player[t].cpu == 2) {
      mag = find_score_by_gameid (game_id);
      if (mag == -1)
	mag = 9;
      if (highs[gamemodeh][mag].points >= player[t].score)
	mag = -1;
      if (mag != -1) {
	enter_your_name (plr2col[t] + 1, highs[gamemodeh][mag].name);
	copy_gameid (highs[gamemodeh][mag].gid, game_id);
	highs[gamemodeh][mag].points = player[t].score;
	sort_scores ();
      }
    }
  write_scores ();
}

static void
output_screen (char n)
{
  pixel_t *src;
  int i;
  int loginf[4];		/* values for counters */

  if (game_mode == M_DEATHM)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].lifes;
  else if (game_mode == M_KILLEM)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].lemmings_nbr;
  else if (game_mode >= M_TCASH)
    for (i = 0; i < 4; i++)
      loginf[i] = player[col2plr[i]].cash;

  if (two_players == false) {
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
      show_txt_bonus (col2plr[0], corner[0] + 40 + 5 * xbuf);
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
      show_txt_bonus (col2plr[0], corner[0] + 1 + 185 * xbuf);
    if (player[col2plr[1]].spec != 0xde)
      show_txt_bonus (col2plr[1], corner[1] + 2 + 185 * xbuf);

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

#define ia_eval_dir_target_inline(s_sens)			\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	    tmp=ia_eval_dir_target(d);				\
	    if (tmp<mindist) mindist=tmp;			\
    }

#define ia_eval_dir_bonus_inline(s_sens)			\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
            tmp=ia_eval_dir_bonus(d);				\
	    if (tmp>mindist) mindist=tmp;			\
    }

#define ia_eval_dir_lemming_inline(s_sens)			\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	    tmp=ia_eval_dir_lemming(d);				\
	    if (tmp>mindist) mindist=tmp;			\
    }

#define ia_eval_dir_cash_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	    tmp=ia_eval_dir_cash(d);				\
	    if (tmp>mindist) mindist=tmp;			\
    }

#define ia_eval_dir_color_inline(s_sens)			\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	    tmp=ia_eval_dir_color(d);				\
	    if (tmp>mindist) mindist=tmp;			\
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
  ia_cur_depth--;
  if (ia_cur_depth != 0) {
    int tmp2 = 0;
    int mindist = 0;
    int d = square2tile[pos];
    int tmp;
    square_occupied[pos] = 128;
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
    int tmp;
    ia_cur_depth++;
    tmp = ia_eval_neighb_pos (w_up, pos) + ia_eval_neighb_pos (w_right, pos)
      + ia_eval_neighb_pos (w_down, pos) + ia_eval_neighb_pos (w_left, pos);
    return (-(tmp << 3));
  }

}

/* give the *way* to follow to get a given position */

#define ia_goto_target_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	  ia_cur_depth=ia_max_depth;				\
	  tmp[s_sens]=ia_eval_dir_target(d);			\
	  if (tmp[s_sens]<mindist) {				\
		mindist=tmp[s_sens];				\
		mindir=s_sens;					\
	  }							\
    }

#define ia_goto_bonus_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	  ia_cur_depth=ia_max_depth;				\
	  tmp[s_sens]=ia_eval_dir_bonus(d);			\
	  if (tmp[s_sens]>mindist) {				\
		mindist=tmp[s_sens];				\
		mindir=s_sens;					\
	  }							\
    }

#define ia_goto_lemming_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	  ia_cur_depth=ia_max_depth;				\
	  tmp[s_sens]=ia_eval_dir_lemming(d);			\
	  if (tmp[s_sens]>mindist) {				\
		mindist=tmp[s_sens];				\
		mindir=s_sens;					\
	  }							\
    }

#define ia_goto_color_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	  ia_cur_depth=ia_max_depth;				\
	  tmp[s_sens]=ia_eval_dir_color(d);			\
	  if (tmp[s_sens]>mindist) {				\
		mindist=tmp[s_sens];				\
		mindir=s_sens;					\
	  }							\
    }

#define ia_goto_cash_inline(s_sens)				\
    d=square_wrap[(pos<<2)+s_sens];				\
    if (d!=-1)							\
    if ((square_occupied[d]==0xff) &&				\
       ((square_explosion[d]>=(NBR_EXPLOSION_FRAMES-1)*8+12) 	\
        || ia_is_invincible)) {					\
	  ia_cur_depth=ia_max_depth;				\
	  tmp[s_sens]=ia_eval_dir_cash(d);			\
	  if (tmp[s_sens]>mindist) {				\
		mindist=tmp[s_sens];				\
		mindir=s_sens;					\
	  }							\
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

    /* Forbid turn back.  This is usually not needed because the
       square behind the vehicle is aleady occupied, but on some
       tunnel configurations this may not be the case. */
    o[player[c].way ^ 2] = c;

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

/* FIXME: there are many things here that should be moved in
   a separate function (e.g. `update_game') in order to be run only
   once per update, and not four times. */
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
	/* KLUGE: mark all players whose time is 0 as dead.  This is
	   needed because many players can reach 0 simultaneously but
	   this block is only run for the first player when this is
	   discovered.  */
	for (i = 0; i < 4; ++i)
	  if (player[i].time == 0) {
	    player[i].spec = 0xde;
	    erase_trail (i);
	  }
	/* find out the richest player and set level_is_finished accordingly */
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
    sprintf (txt_tmp, _("INVERTED %d"), player[c].inversed_controls / 20 + 1);
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
    sprintf (txt_tmp, _("STOPPED %d"), player[c].delay / 20 + 1);
    set_txt_bonus (c, txt_tmp, 2);
  }


  if (player[c].d.h.h != 0 || player[c].delay == 1) {

/**** handling of trails ****/
    if (player[c].delay == 0) {
      l = player[c].x2 + player[c].y2 * map_info_2xt;
      square_occupied[l] = (char) (c + 8);
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

    d2 = player[c].y2 * map_info_2xt + player[c].x2;
    if (player[c].delay == 0)
      d2 = square_wrap[(d2 << 2) + player[c].way];
    player[c].pos = d2;
    player[c].x2 = square_offset2coord[d2 << 1];
    player[c].y2 = square_offset2coord[(d2 << 1) + 1];
    player[c].d.h.h = 0;

    if (player[c].spec == t_tunnel) {
      player[c].way = player[c].tunnel_way;
      player[c].old_way = player[c].way;
      player[c].spec = 0;
    }

    if (cpuon) {
      if (player[c].target < 16) {
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
    square_occupied[d2] = c;
    if ((square_explosion[d2] <= (NBR_EXPLOSION_FRAMES - 1) * 8 - 1) &&
	player[c].invincible == 0)
      player[c].spec = 0xff;

    player[c].square = (char) ((player[c].x2 & 1) + (player[c].y2 & 1) * 2);
    d = (player[c].x2 >> 1) + (player[c].y2 >> 1) * map_info.xt;

    if (level_map[d].type == t_ice
	&& level_map[d].info.param[player[c].square] != 0)
      player[c].spec = t_ice;
    if ((level_map[d].type == t_stop
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
	    (char *) square_dead_lemmings_list[i];
	  square_dead_lemmings_list[i] = tmppti;
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
	    else if (t == 8 + c)
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
	if (player[(i + 1) & 3].lifes == 0
	    && player[(i + 2) & 3].lifes == 0
	    && player[(i + 3) & 3].lifes == 0)
	  level_is_finished = i + 1;
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
    player[c].old_old_way = player[c].old_way;
    player[c].old_way = player[c].way;

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
	} else {
	  if (player[c].cpu == 2)
	    event_sfx (61);
	}
	/* TRANS: %d, the number of remaining lives, is always
	   positive.  */
	sprintf (txt_tmp, ngettext("LAST LIFE", "%d LIVES LEFT",
				   player[c].lifes), player[c].lifes);
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
      if (square_explosion[d2] ==
	  255) {
	square_explosion[d2] =
	  200;
	square_explosion_type[d2] = rand () % NBR_EXPLOSION_KINDS;
      }
    }
    player[c].delay = 0;

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
	square_explosion_type[x2] = rand () % NBR_EXPLOSION_KINDS;
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = rand () % NBR_EXPLOSION_KINDS;
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = rand () % NBR_EXPLOSION_KINDS;
      }
      x2 = *m++;
      if (x2 >= 0 && (square_explosion[x2] == 255)) {
	square_explosion[x2] = 200;
	square_explosion_type[x2] = rand () % NBR_EXPLOSION_KINDS;
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

  if (two_players == false) {
    int flip_pos;
    pendulum_init ();
    n = 0;
    do {
      /* FIXME: use a timer for the pendulumn */
      flip_pos = pendulum_update (n);
      flip_buffer (flip_pos);
      flush_display (render_buffer[1]);
      output_screen ((char) n);
      n = update_all (0);
    } while (elapsed_time < 3000);
  }
  if (two_players) {
    int buffer_pos = 39;
    do {
      flush_display2_moving (buffer_pos);
      output_screen ((char) n);
      n = update_all (0);
      for (i = n; i > 0; i--)
	if (buffer_pos > 0)
	  buffer_pos--;
    } while (buffer_pos != 0);
  }

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
      if (two_players == false) {
	flush_display (corner[0]);
      } else {
	flush_display2 (corner[0], corner[1]);
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
      flush_display (corner[0]);
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
	    flush_display (corner[0]);
	  }
	  load_demo ();
	  dmsg (D_SECTION, "-- (back to) menu (from demo) --");
	  demo_ready = 0;
	  event_sfx (131);
	  std_white_fadein (&tile_set_img.palette);
	  pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
	  {
	    pixel_t *pixbuf;
	    XMALLOC_ARRAY (pixbuf, xbuf * 200);
	    do {
	      background_menu ();
	      draw_main_menu (l);
	      pixelize_pos = read_htimer (pixelize_timer);
	      if (pixelize_pos > 6)
		pixelize_pos = 6;
	      pixelize[6 - pixelize_pos] (pixbuf, corner[0]);
	      flush_display (pixbuf);
	    } while (pixelize_pos < 6);
	    free (pixbuf);
	  }
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
      save_preferences ();
    }
    if (l == 2)
      help_menu ();
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
    htimer_t pixelize_timer;
    long pixelize_pos;
    fader_status_t fade_stat;
    pixel_t *pixbuf;
    XMALLOC_ARRAY (pixbuf, 200 * xbuf);
    pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
    std_black_fadeout (&tile_set_img.palette);
    fader_status_flagback (&fade_stat);
    do {
      background_menu ();
      draw_quit_menu (0);
      pixelize_pos = read_htimer (pixelize_timer);
      if (pixelize_pos > 6)
	pixelize_pos = 6;
      pixelize[pixelize_pos] (pixbuf, corner[0]);
      flush_display (pixbuf);
    } while (fade_stat != F_FINISHED);
    free (pixbuf);
    free_htimer (pixelize_timer);
  }
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
  if (two_players == true) {
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
  sprite_t *levelname;

  dmsg (D_SECTION, "-- play game --");

  in_menu = 0;
  tutor = (char) (game_mode == M_QUEST && current_quest_level == 0);
  if (loadulevel == 1) {
    char* tmp = get_non_null_rsc_file ("levels-dir");
    strappend (tmp, level_name);
    if (load_level (level_name, cont))
      emsg (_("Error during level loading"));
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
    sprintf (bufstr, _("-LEVEL %d-"), current_quest_level);
  else
    sprintf (bufstr, _("-ROUND %d/%d-"),
	     rounds_nbr_values[opt.gamerounds] - rounds + 1,
	     rounds_nbr_values[opt.gamerounds]);

  levelname = compile_menu_text (bufstr, T_CENTERED, 99, 159);

  if (two_players == false) {
    int pendulum_pos;
    pendulum_init ();
    n = 0;
    do {
      pendulum_pos = pendulum_update (n);
      flip_buffer (pendulum_pos);

      DRAW_SPRITE (levelname, render_buffer[1]);

      flush_display (render_buffer[1]);
      output_screen ((char) n);
      process_input_events ();
      n = update_all (0);
    } while (elapsed_time < 3000);
  }
  if (two_players) {
    int buffer_pos = 39;

    corner[0] = render_buffer[0];
    clear_scr_area (corner[0]);
    DRAW_SPRITE (levelname, corner[0]);
    flush_display (corner[0]);

    sleep (1);
    update_htimers ();
    reset_htimer (update_htimer);

    n = 1;
    do {
      DRAW_SPRITE (levelname, corner[swapside] + (buffer_pos << 2));
      DRAW_SPRITE (levelname, corner[1 - swapside] - (buffer_pos << 2) - 160);
      flush_display2_moving (buffer_pos);
      output_screen ((char) n);
      process_input_events ();
      n = update_all (0);
      for (i = n; i > 0; i--)
	if (buffer_pos > 0)
	  buffer_pos--;
    } while (buffer_pos != 0);

  }
  event_sfx (141);

  free_sprite (levelname);

/* * * * * * * * * * * * * * *\
* MAIN LOOP during the games *
\* * * * * * * * * * * * * * */

  dmsg (D_SECTION, "game main loop");

  do {
    event_time = read_htimer (event_htimer);
    update_text_waving_step ();
    if (enable_blit) {
      if (two_players == false) {
	flush_display (corner[0]);
      } else {
	flush_display2 (corner[0], corner[1]);
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
      event_time = read_htimer (event_htimer);
      update_text_waving_step ();
      if (two_players == false) {
	draw_end_level_info (0, l);
	flush_display (corner[0]);
      } else {
	pixel_t* tmp;

	tmp = corner[0];
	draw_end_level_info (swapside ? -160 : 0, l);
	corner[0] = corner[1];
	draw_end_level_info (swapside ? 0 : -160, l);
	corner[0] = tmp;
	flush_display2 (corner[0], corner[1]);
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
	/* FIXME: The whole process here need to be rethought, keeping
	   in mind that several process can access this save file.  Presently
	   the file is locked until the menu exits, this is bad because
	   other processes will block until the user eventually exits
	   the menu.  */
	load_save_records_and_keep_locked ();
	t = 0;
	l = 0;
	editflag = 0;
	do {
	  if (two_players == false) {
	    draw_saved_games_info (0, l, true);
	    flush_display (corner[0]);
	  } else {
	    pixel_t *tmp;

	    tmp = corner[0];
	    draw_saved_games_info (swapside ? -160 : 0, l, true);
	    corner[0] = corner[1];
	    draw_saved_games_info (swapside ? 0 : -160, l, true);
	    corner[0] = tmp;
	    flush_display2 (corner[0], corner[1]);
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
	      if (t == 0x0e7f) { /* FIXME: choose a keysym to use */
		saverec[l].used = 0;
		saverec[l].name[0] = 0;
		event_sfx (128);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
	      }
	      if (t == HK_Enter) {
		editflag = 1;
		strcpy (tmpname, saverec[l].name);
		pos = strlen (saverec[l].name);
		saverec[l].name[pos] = '^';
		saverec[l].name[pos + 1] = 0;
		event_sfx (125);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
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
		  FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
		}
	      if ((t == HK_BackSpace || t == HK_Delete) && (pos > 1)) {
		saverec[l].name[pos - 1] = 0;
		saverec[l].name[pos - 2] = '^';
		event_sfx (122);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
	      } else if (t == 0x1c0a) { /* FIXME: choose a keysym to use */
		saverec[l].name[pos - 1] = 0;
		event_sfx (127);
		editflag = 2;
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
	      } else if (t == HK_Escape) {
		strcpy (saverec[l].name, tmpname);
		event_sfx (123);
		editflag = 2;
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
	      } else if (t == HK_Enter) {
		saverec[l].name[pos - 1] = 0;
		saverec[l].level = current_quest_level /*+1 */ ;
/*                saverec[l].questmode=questmode; */
		for (u = 0; u < 4; u++) {
		  saverec[l].points[u] = player[col2plr[u]].score;
		  copy_gameid (saverec[l].gid, game_id);
		  saverec[l].lifes[u] = player[col2plr[u]].lifes;
		}
		saverec[l].used = 1;
		editflag = 0;
		event_sfx (124);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
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
	event_time = read_htimer (event_htimer);
	update_text_waving_step ();
	if (two_players == false) {
	  draw_end_level_info (0, l);
	  flush_display_moving (i);
	} else {
	  pixel_t *tmp;

	  tmp = corner[0];
	  draw_end_level_info (swapside ? -160 : 0, l);
	  corner[0] = corner[1];
	  draw_end_level_info (swapside ? 0 : -160, l);
	  corner[0] = tmp;
	  flush_display2_moving (i);
	}
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
      }
      if (two_players == false) {
	draw_end_level_info (0, l);
	flush_display_moving (40);
      } else {
	pixel_t *tmp;
	tmp = corner[0];
	draw_end_level_info (swapside ? -160 : 0, l);
	corner[0] = corner[1];
	draw_end_level_info (swapside ? 0 : -160, l);
	corner[0] = tmp;
	flush_display2_moving (40);
      }
    } else {
      dmsg (D_SECTION, "print round info");

      l = 0;
      flag = 1;

      event_sfx (129);
      do {
	event_time = read_htimer (event_htimer);
	update_text_waving_step ();
	if (two_players == false) {
	  draw_round_info (0);
	  flush_display (corner[0]);
	} else {
	  pixel_t *tmp;
	  tmp = corner[0];
	  draw_round_info (swapside ? -160 : 0);
	  corner[0] = corner[1];
	  draw_round_info (swapside ? 0 : -160);
	  corner[0] = tmp;
	  flush_display2 (corner[0], corner[1]);
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
	event_time = read_htimer (event_htimer);
	update_text_waving_step ();
	if (two_players == false) {
	  draw_round_info (0);
	  flush_display_moving (i);
	} else {
	  pixel_t *tmp;
	  tmp = corner[0];
	  draw_round_info (swapside ? -160 : 0);
	  corner[0] = corner[1];
	  draw_round_info (swapside ? 0 : -160);
	  corner[0] = tmp;
	  flush_display2_moving (i);
	}
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
      }
      if (two_players == false) {
	draw_round_info (0);
	flush_display_moving (40);
      } else {
	pixel_t *tmp;
	tmp = corner[0];
	draw_round_info (swapside ? -160 : 0);
	corner[0] = corner[1];
	draw_round_info (swapside ? 0 : -160);
	corner[0] = tmp;
	flush_display2_moving (40);
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
    emsg (_("Cannot open %s."), t);
  }
  free (t);
  while (!feof (f)) {
    fgets ((char *) string, 32, f);
    if (string[0] == '>' || string[0] == ' ')
      level_list_nbr++;
  }
  fseek (f, 0, 0);
  XMALLOC_ARRAY (level_list, level_list_nbr * 13);
  XMALLOC_ARRAY (levelinf, level_list_nbr);
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

int
main (int argc, char *argv[])
{
  int i;

  mtrace (); /* GNU libc's malloc debugging facility */

  dmsg_init (argv[0]);
  dmsg (D_SECTION, "initialization");

  var_initialize ();		/* Needed by init_persona.  */
  init_persona ();

  relocate_data ();

  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, get_non_null_rsc_file ("locale-dir"));
  textdomain (PACKAGE);


  init_sound_track_list ();

  /* Read the system-wide configuration file. */
  {
    char* tmp;
    bool sec = true;
    tmp = get_rsc_file_secure ("system-conf", &sec);
    if (tmp) {
      read_userconf (tmp, sec);
      free (tmp);
    }
  }

  if (setup_userdir ())
    exit (1);

  add_default_extra_directories ();

  /* Read the user configuration file. */
  {
    char* tmp;
    bool sec = true;
    tmp = get_rsc_file_secure ("user-conf", &sec);
    if (tmp) {
      read_userconf (tmp, sec);
      free (tmp);
    }
  }

  freeze_sound_track_list ();

  dmsg (D_SYSTEM, "parsing command line");
  if (parse_argv (argc, argv, 0, 0))
    exit (1);

  dmsg (D_SYSTEM, "randomize");
  srand (time (0));

  read_level_list ();

  browse_extra_directories ();
  if (reinitopt)
    reinit_preferences ();
  else
    load_preferences ();
  if (showprefs) {
    output_preferences (stdout);
    exit (0);
  }

  if (reinitsco) {
    clear_scores ();
    write_scores ();
  }

  if (reinitsav) {
    clear_save_records ();
    write_save_records ();
  }

  if (read_sfx_conf ())
    emsg (_("error in sfx.cfg"));

  if (joyoff) {
    joystick_detected = 0;
    /* reset controlers configuration to keyboards */
    opt.ctrl_one = 0;
    opt.ctrl_two = 0;
  } else
    joyinit ();

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
  pcx_load_from_rsc ("vehicles-img", &vehicles_img);
  pcx_load_from_rsc ("jukebox-font", &font_deck_img);

  init_fonts ();
  init_explosions ();
  init_items ();

  for (i = 0; i != 32; i++)
    minisinus[i] = ceil (sin (i * 2.0 * 3.141592653 / 32.0) * 1.7);
  compute_lut ();

  init_menus_sprites ();
  dummy_moving_background_init ();
  update_htimers ();
  main_menu ();

  dummy_moving_background_uninit ();
  uninit_menus_sprites ();
  uninit_fader ();
  uninit_text_waving_step ();

  uninit_items ();
  uninit_explosions ();
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
  img_free (&vehicles_img);
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
  var_uninitialize ();
  uninit_sound_track_list ();
  free_extra_list ();
  free_extra_directories ();
  save_preferences ();
  free_save_records ();
  free_scores ();
  free_preferences ();
  free_userdir ();
  free_modified_rsc ();
  return 0;
}
