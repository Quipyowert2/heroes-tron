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

#include "system.h"
#include "video.h"
#include "pcx.h"
#include "keyb.h"
#include "keyvalues.h"
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

#include "strack.h"
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
#include "camera.h"
#include "levellst.h"
#include "locales.h"
#include "main.h"
#include "pendulum.h"
#include "hookscore.h"
#include "plugins.h"

char tile_set_name[128];
char glenz_name[128];

int *level_full_list;
size_t level_full_list_size = 0;
int rounds = 1;

unsigned int current_quest_level;

#define DEMO_DURATION 90

char enable_blit;

char mouse_found = 1;
/***********************/

char txt_tmp[20];

a_timer blink_htimer;
a_timer clock_htimer;
a_timer tiles_anim_htimer;
a_timer corner_htimer;
a_timer event_htimer;
long event_time;		/* updated from event_htimer on each frame */
a_timer waving_htimer;
a_timer background_htimer;
a_timer sound_track_htimer;
a_timer demo_trigger_htimer;

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

static char
load_level (char *filename, char cont)
{
  int err;

  dmsg (D_FILE|D_LEVEL, "loading level: %s", filename);

  if (!in_menu) {
    if (two_players) {
      state.player[state.col2plr[0]].cpu = 2;
      state.player[state.col2plr[1]].cpu = 2;
      state.player[state.col2plr[2]].cpu = 0;
      state.player[state.col2plr[3]].cpu = 0;
    } else {
      state.player[state.col2plr[0]].cpu = 2;
      state.player[state.col2plr[1]].cpu = 0;
      state.player[state.col2plr[2]].cpu = 0;
      state.player[state.col2plr[3]].cpu = 0;
    }
  }
  if (in_demo) {
    state.player[state.col2plr[0]].cpu = 0;
    state.player[state.col2plr[1]].cpu = 0;
  }

  clean_buffers ();

  err = lvl_load_file (filename, &lvl, true);
  if (err) {
    dmsg (D_LEVEL|D_FILE, "cannot open %s", filename);
    dperror ("lvl_load_file");
    return err;
  }

  dmsg (D_LEVEL, "size=(%u,%u) wrap=(%x,%x) tile=%s soundtrack=%s",
	lvl.tile_width, lvl.tile_height,
	lvl.tile_width_wrap, lvl.tile_height_wrap,
	lvl_tile_sprite_map_basename (&lvl), lvl_sound_track (&lvl));

  {
    const char *bn = lvl_tile_sprite_map_basename (&lvl);
    int fd;
    char* tmp = get_non_null_rsc_file ("tiles-sets-dir");
    stpcpy (stpcpy (stpcpy (tile_set_name, tmp), bn), ".pcx");
    stpcpy (stpcpy (stpcpy (glenz_name, tmp), bn), ".glz");
    free (tmp);

    pcx_load (tile_set_name, &tile_set_img);

    dmsg (D_LEVEL|D_FILE, "loading %s", glenz_name);
    fd = open (glenz_name, O_RDONLY | O_BINARY);
    if (fd == -1) {
      dperror ("open");
      return errno;
    }

    if (read (fd, glenz, 256 * 8) != 256 * 8) {
      dperror ("read");
      return errno;
    }
    close (fd);
  }

  init_render_data ();

  if (in_menu) {
    load_soundtrack_from_alias ("MENU");
  } else {
    load_soundtrack_from_alias (lvl_sound_track (&lvl));
  }

  dmsg (D_LEVEL,
	"initialize variables and maps associated to the level, (mode %d).",
	state.game_mode);

  state_init (&state, &lvl, cont, two_players, in_menu);
  return (0);
}

static void
unload_level (void)
{
  dmsg (D_LEVEL, "unloading level");

  uninit_render_data ();
  img_free (&tile_set_img);
  state_free (&state);
  lvl_free (&lvl);
  unload_soundtrack ();
}

static void
load_level_from_number (int nbr, char cont)
{
  char e;

  dmsg (D_SECTION, "load level #%d", nbr);
  e = load_level (level_list[nbr].name, cont);
  if (e != 0)
    emsg (_("Cannot load level %s (error %d)"), level_list[nbr].name, e);
}

static void
compute_level_full_list (void)
{
  int i;
  unsigned int j;

  dmsg (D_SECTION, "compute level full list");

  level_full_list_size = level_list_size + extra_nbr;
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
  }
  i = 0;
  if (opt.extras != 2)
    for (; i < (int) level_list_size; i++)
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
  unsigned int i, j, k;
  assert (level_full_list_size > 0);
  i = rand () % level_full_list_size;
  level_full_list_size--;
  k = level_full_list[i];
  for (j = i; j < level_full_list_size; j++)
    level_full_list[j] = level_full_list[j + 1];
  return (k);
}

static void
load_random_wrapped_level (bool wrapped, char cont)
{
  int t;

  dmsg (D_SECTION, "load random wrapped level");

  do {
    t = rand () % level_list_size;
  } while (wrapped && !level_list[t].wrapped);

  load_level_from_number (t, cont);
}

static void
load_random_level (char cont)
{
  int t;
  char *tmp;
  char e;
  t = random_level ();

  dmsg (D_SECTION, "load random level");

  if (t & 0x10000) {
    tmp = extra_list[t & 0xffff].full_name;
  } else {
    tmp = level_list[t].name;
  }
  e = load_level (tmp, cont);
  if (e != 0)
    emsg (_("Cannot load level %s (error %d)"), tmp, e);
}

static void
play_menu (void)
{
  char cont;
  FILE *ftmp;
  int gamemodeh;
  static int l = 1, u = 0;
  char flagload = 0;
  a_keycode t;
  int i;
  a_timer flip_timer;
  long flip_pos;

  if (l == 5)
    l = 1;

  for (i = 3; i >= 0; i--)
    state_set_player_color (&state, opt.player_color[i], i);

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
      state.game_mode = M_QUEST;
    else if (l == 2)
      state.game_mode = M_DEATHM;
    else if (l == 3)
      state.game_mode = M_KILLEM;
    else if (l == 4)
      state.game_mode = M_TCASH;
    else if (l == 5)
      state.game_mode = M_COLOR;
    gamemodeh = state.game_mode;
    cont = 0;
    current_quest_level = 0;
    create_gameid (game_id);
  } else {
    gamemodeh = state.game_mode = M_QUEST;
    current_quest_level = saverec[u].level;
    copy_gameid (game_id, saverec[u].gid);
    for (t = 0; t < 4; t++) {
      state.player[state.col2plr[t]].lifes = saverec[u].lifes[t];
      state.player[state.col2plr[t]].score = saverec[u].points[t];
    }
    cont = 1;
  }

  unload_level ();		/* also stop the music */

  if (opt.sfx)
    load_sfx_mode (state.game_mode);
  if (state.game_mode > M_QUEST) {
    compute_level_full_list ();
    rounds = rounds_nbr_values[opt.gamerounds];
  } else
    rounds = 1;
  while (((current_quest_level < level_list_size)
	  || ((state.game_mode > M_QUEST)
	      && (current_quest_level < level_full_list_size)))
	 && (rounds > 0) && (play_game (cont) == 0)) {
    cont = 1;
    if (state.game_mode > M_QUEST) {
      rounds--;
      if (level_full_list_size == 0)
	rounds = 0;
    }
  }

  if (state.game_mode > M_QUEST)
    free_level_full_list ();

  /* END OF THE GAME */

  if ((state.game_mode == M_QUEST)
      && (current_quest_level >= level_list_size)) {
    if (state_level_exit_code (&state) != 15) {
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

  state.game_mode = M_QUEST;
/* free_all_sfx(); */
  load_random_wrapped_level (true, cont);
  load_sfx_mode (-1);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  /* The logic is quite convolued here because we instert the score in
     a file which might be shared by several process.

     First we load the scores (to get up-to-date informations) and
     then if the current score deserve an insertion we ask for the
     player's name, and finally lock the file for insertion.

     Note that if the scores file is updated while the player enter
     his names, the current scores might not deserve an insertion in
     the file anymore.  The only consequence is that the player has
     entered his name for nothing.  */
  load_scores ();
  for (t = 0; t < 4; t++)
    if (state.player[t].cpu == 2) {
      if (insert_scores (gamemodeh, 0, game_id, state.player[t].score)) {
	char player_name[PLAYER_NAME_SIZE + 1];
	enter_your_name (state.plr2col[t] + 1, player_name);
	load_scores_and_keep_locked ();
	insert_scores (gamemodeh, player_name, game_id, state.player[t].score);
	write_scores_locked ();
      }
    }
}

/* These variables are used to handle the moving radar and score at
   the begging of a level.  */
int radar_target_pos;
int radar_current_pos;

static void
output_screen (char n)
{
  a_pixel *src;
  int i;
  int loginf[4];		/* values for counters */

  position_camera ();

  if (state.game_mode == M_DEATHM)
    for (i = 0; i < 4; i++)
      loginf[i] = state.player[state.col2plr[i]].lifes;
  else if (state.game_mode == M_KILLEM)
    for (i = 0; i < 4; i++)
      loginf[i] = state.player[state.col2plr[i]].lemmings_nbr;
  else if (state.game_mode >= M_TCASH)
    for (i = 0; i < 4; i++)
      loginf[i] = state.player[state.col2plr[i]].cash;

  if (two_players == false) {
    compute_corner (0, n);
    draw_level (0);
    if (state.player[state.col2plr[0]].waves) {
      wave_buffer ();
      corner[0] = render_buffer[0];
    }
    if (state.player[state.col2plr[0]].rotozoom) {
      rotozoom_buffer ();
      corner[0] = render_buffer[1] + xbuf;
    }
    if (opt.radar_map)
      draw_radar_map (state.player[state.col2plr[0]].x2, state.player[state.col2plr[0]].y2,
		      radar_current_pos);
    if (opt.display_infos)
      draw_score (state.col2plr[0], 0, 5, 5, -radar_current_pos);
    if (state.game_mode != M_QUEST) {
      if (radar_current_pos <= 70)
	draw_logo_info (state.col2plr[0], loginf[0],
			corner[0] + (183 + ((radar_current_pos >= 30)
					    ? ((radar_current_pos - 30) >> 1)
					    : 0)) * xbuf + 24);
      if (radar_current_pos <= 60)
	draw_logo_info (state.col2plr[1], loginf[1],
			corner[0] + (183 + ((radar_current_pos >= 20)
					    ? ((radar_current_pos - 20) >> 1)
					    : 0)) * xbuf + 98);
      if (radar_current_pos <= 50)
	draw_logo_info (state.col2plr[2], loginf[2],
			corner[0] + (183 + ((radar_current_pos >= 10)
					    ? ((radar_current_pos - 10) >> 1)
					    : 0)) * xbuf + 172);
      if (radar_current_pos <= 40)
	draw_logo_info (state.col2plr[3], loginf[3],
			corner[0] + (183 + (radar_current_pos >> 1)) * xbuf +
			246);
    }
    if (state.player[state.col2plr[0]].spec != 0xde)
      show_txt_bonus (state.col2plr[0], corner[0] + 40 + 5 * xbuf);
  } else {
    compute_corner (0, n);
    compute_corner (1, n);
    draw_level (0);
    draw_level (1);
    if (state_level_exit_code (&state) == 0) {
      if (state.player[state.col2plr[0]].waves) {
	wave_half_buffer (0);
	corner[0] = render_buffer[0];
      }
      if (state.player[state.col2plr[0]].rotozoom) {
	rotozoom_half_buffer (0);
	corner[0] = render_buffer[0] + xbuf - 180 + xbuf;
      }
      if (state.player[state.col2plr[1]].waves) {
	wave_half_buffer (1);
	corner[1] = render_buffer[1];
      }
      if (state.player[state.col2plr[1]].rotozoom) {
	rotozoom_half_buffer (1);
	corner[1] = render_buffer[1] + xbuf - 180 + xbuf;
      }
    }
    if (state.game_mode != M_QUEST) {
      if (radar_current_pos <= 60)
	draw_logo_info (state.col2plr[0], loginf[0],
			corner[swapside] + (5 - ((radar_current_pos >= 30)
						 ? ((radar_current_pos - 30)
						    >> 1) : 0)) * xbuf + 44);
      if (radar_current_pos <= 50)
	draw_logo_info (state.col2plr[1], loginf[1],
			corner[swapside] + (5 - ((radar_current_pos >= 20)
						 ? ((radar_current_pos - 20)
						    >> 1) : 0)) * xbuf + 104);
      if (radar_current_pos <= 40)
	draw_logo_info (state.col2plr[2], loginf[2],
			corner[1 - swapside] + (5 - ((radar_current_pos >= 10)
						     ? (
							(radar_current_pos -
							 10) >> 1) : 0)) *
			xbuf + 6);
      if (radar_current_pos <= 30)
	draw_logo_info (state.col2plr[3], loginf[3],
			corner[1 - swapside] + (5 -
						(radar_current_pos >> 1)) *
			xbuf + 66);
    }
    if (state.player[state.col2plr[0]].spec != 0xde)
      show_txt_bonus (state.col2plr[0], corner[0] + 1 + 185 * xbuf);
    if (state.player[state.col2plr[1]].spec != 0xde)
      show_txt_bonus (state.col2plr[1], corner[1] + 2 + 185 * xbuf);

    src = corner[swapside] + 158;
    for (i = 200; i != 0; i--) {
      *src = glenz[0][(int) *src];
      ++src;
      *src = glenz[0][(int) glenz[0][(int) *src]];
      src += xbuf - 1;
    }

    src = corner[1 - swapside];
    for (i = 200; i != 0; i--) {
      *src = glenz[0][(int) glenz[0][(int) *src]];
      ++src;
      *src = glenz[0][(int) *src];
      src += xbuf - 1;
    }

    if (opt.display_infos) {
      if (swapside) {
	draw_score (state.col2plr[0], 0, 5, 122, radar_current_pos);
	draw_score (state.col2plr[1], 1, 5, 5, -radar_current_pos);
      } else {
	draw_score (state.col2plr[0], 0, 5, 5, -radar_current_pos);
	draw_score (state.col2plr[1], 1, 5, 122, radar_current_pos);
      }
    }
  }
}

/****************/

static int
update_all (int started)
{
  int frames = state_update (&state);

  if (started) {
    int n;
    for (n = 0; n < frames; ++n) {
      if (radar_current_pos < radar_target_pos)
	radar_current_pos++;
      else if (radar_current_pos > radar_target_pos)
	radar_current_pos--;
    }
  }

  return frames;
}


void
play_demo (void)
{
  int n, i;
  char notbyebye = 1;
  a_timer demo_htimer = new_htimer (T_GLOBAL, HZ (1));
  a_fader_status fade_stat = F_UNKNOWN;

  dmsg (D_SECTION, "-- play demo --");

  in_menu = 0;
  tutor = 0;
  load_random_wrapped_level (false, 0);

  play_soundtrack ();
  set_pal_with_luminance (&tile_set_img.palette);
  radar_current_pos = 81;
  radar_target_pos = 0;
/* .... game .... */
  if (two_players) {
    nbr_tiles_cols = 8;
  }
  init_camera ();
  init_keyboard_map ();
  n = 1;

  update_htimers ();
  reset_htimer_with_offset (event_htimer, 4);
  reset_htimer (clock_htimer);
  reset_htimer (bonus_anim_htimer);
  reset_htimer (tiles_anim_htimer);
  reset_htimer (blink_htimer);
  state_start_game (&state);
  output_screen ((char) n);	/* update corner[] */
/*   corner[0]=render_buffer[0]; */
/*   corner[1]=render_buffer[1]; */

  if (two_players == false) {
    int flip_pos;
    a_pendulum *p = pendulum_create ();
    n = 0;
    while (pendulum_update (p, &flip_pos) < 3000) {
      flip_buffer (flip_pos);
      flush_display (render_buffer[1]);
      output_screen ((char) n);
      n = update_all (0);
    }
    pendulum_destroy (p);
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
  state_start_players (&state);

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
    if (state_level_exit_code (&state) == 0)
      n = update_all (1);

    if (devparm && keyboard_map[HK_F12])
      state_level_set_exit_code (&state, 1);

    if (opt.ctrl_one ^ opt.ctrl_two)
      if (is_joystick_button_b (1) && enable_blit)
	notbyebye = 0;

    if (keyboard_map[HK_Escape] && enable_blit)
      notbyebye = 0;

    if (!(notbyebye && state_level_exit_code (&state) == 0)
	&& fade_stat == F_UNKNOWN) {
      /* startup a black fade-out */
      std_black_fadeout (&tile_set_img.palette);
      fader_status_flagback (&fade_stat);
    }
  } while (fade_stat != F_FINISHED);

  free_htimer (demo_htimer);
  uninit_keyboard_map ();
  unload_level ();
  nbr_tiles_cols = 15;
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
    state.col2plr[i] = opt.player_color[i];
  for (i = 3; i >= 0; i--)
    state.plr2col[opt.player_color[i]] = i;
  gamemodeh = (rand () & 3) + 2;
  olddeuxplr = two_players;
  two_players = rand () & 1;
  if (gamemodeh == 2)
    state.game_mode = M_DEATHM;
  else if (gamemodeh == 3)
    state.game_mode = M_KILLEM;
  else if (gamemodeh == 4)
    state.game_mode = M_TCASH;
  else if (gamemodeh == 5)
    state.game_mode = M_COLOR;
  current_quest_level = 0;

  unload_level ();		/* stop also the music */
  in_demo = 1;
  if (opt.sfx)
    load_sfx_mode (state.game_mode);
  rounds = 1;
  play_demo ();
  state.game_mode = M_QUEST;
  in_demo = 0;
  two_players = olddeuxplr;
  load_random_wrapped_level (true, 0);
  load_sfx_mode (-1);

  play_soundtrack ();
  reset_htimer (sound_track_htimer);
  reset_htimer (demo_trigger_htimer);
}

static void
main_menu (void)
{
  static char l = 0;
  a_keycode t;
  char flag = 0;

  dmsg (D_SECTION, "-- menu --");

  load_random_wrapped_level (true, 0);
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
	  a_timer pixelize_timer;
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

	  if (check_what == check_demo)
	    exit_heroes (0);

	  dmsg (D_SECTION, "-- (back to) menu (from demo) --");
	  demo_ready = 0;
	  event_sfx (131);
	  std_white_fadein (&tile_set_img.palette);
	  pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
	  {
	    a_pixel *pixbuf;
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
    a_timer pixelize_timer;
    long pixelize_pos;
    a_fader_status fade_stat;
    a_pixel *pixbuf;
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

    if (keyboard_map[opt.player_keys[0][0]]) {
      state.player[state.col2plr[0]].next_way = w_up;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][1]]) {
      state.player[state.col2plr[0]].next_way = w_left;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][2]]) {
      state.player[state.col2plr[0]].next_way = w_down;
      flag1 = 1;
    }
    if (keyboard_map[opt.player_keys[0][3]]) {
      state.player[state.col2plr[0]].next_way = w_right;
      flag1 = 1;
    }
    state.player[state.col2plr[0]].turbo = keyboard_map[opt.player_keys[0][4]] ? 2 : 1;
    if (keyboard_map[opt.player_keys[0][5]]) {
      if (state.player[state.col2plr[0]].turbo == 2) {
	/* two buttons pushed */
	state.player[state.col2plr[0]].turbo = 1;
      } else
	state.player[state.col2plr[0]].turbo = 0;
    }
  } else {
    joystick_x[0] = 0;
    joystick_y[0] = 0;
    joystick_x[1] = 0;
    joystick_y[1] = 0;
    get_joystick_state ();
    if (is_joystick_up (0)) {
      state.player[state.col2plr[0]].next_way = w_up;
      flag1 = 1;
    }
    if (is_joystick_left (0)) {
      state.player[state.col2plr[0]].next_way = w_left;
      flag1 = 1;
    }
    if (is_joystick_down (0)) {
      state.player[state.col2plr[0]].next_way = w_down;
      flag1 = 1;
    }
    if (is_joystick_right (0)) {
      state.player[state.col2plr[0]].next_way = w_right;
      flag1 = 1;
    }
    state.player[state.col2plr[0]].turbo = (is_joystick_button_a (0) ? 2 : 1);
    if (is_joystick_button_b (0)) {
      if (state.player[state.col2plr[0]].turbo == 2) {
	/* two buttons pushed */
	state.player[state.col2plr[0]].turbo = 1;
      } else
	state.player[state.col2plr[0]].turbo = 0;
    }
  }

  if (state.player[state.col2plr[0]].inversed_controls != 0) {
    if (flag1)
      state.player[state.col2plr[0]].next_way ^= 2;
    state.player[state.col2plr[0]].tunnel_inverse = 1;
  } else
    state.player[state.col2plr[0]].tunnel_inverse = 0;
  if (((state.player[state.col2plr[0]].next_way ^ 2) == state.player[state.col2plr[0]].tunnel_way)
      && (state.player[state.col2plr[0]].spec == t_tunnel))
    state.player[state.col2plr[0]].next_way = state.player[state.col2plr[0]].tunnel_way;
  if (two_players == true) {
    if (opt.ctrl_two == 0) {
      if (keyboard_map[opt.player_keys[1][0]]) {
	state.player[state.col2plr[1]].next_way = w_up;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][1]]) {
	state.player[state.col2plr[1]].next_way = w_left;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][2]]) {
	state.player[state.col2plr[1]].next_way = w_down;
	flag2 = 1;
      }
      if (keyboard_map[opt.player_keys[1][3]]) {
	state.player[state.col2plr[1]].next_way = w_right;
	flag2 = 1;
      }
      state.player[state.col2plr[1]].turbo = keyboard_map[opt.player_keys[1][4]] ? 2 : 1;
      if (keyboard_map[opt.player_keys[1][5]]) {
	if (state.player[state.col2plr[1]].turbo == 2) {
	  /* two buttons pushed */
	  state.player[state.col2plr[1]].turbo = 1;
	} else
	  state.player[state.col2plr[1]].turbo = 0;
      }
    } else {
      if (!j) {
	joystick_x[0] = 0;
	joystick_y[0] = 0;
	get_joystick_state ();
      }
      if (is_joystick_up (j)) {
	state.player[state.col2plr[1]].next_way = w_up;
	flag2 = 1;
      }
      if (is_joystick_left (j)) {
	state.player[state.col2plr[1]].next_way = w_left;
	flag2 = 1;
      }
      if (is_joystick_down (j)) {
	state.player[state.col2plr[1]].next_way = w_down;
	flag2 = 1;
      }
      if (is_joystick_right (j)) {
	state.player[state.col2plr[1]].next_way = w_right;
	flag2 = 1;
      }
      state.player[state.col2plr[1]].turbo = (is_joystick_button_a (j) ? 2 : 1);
      if (is_joystick_button_b (j)) {
	if (state.player[state.col2plr[1]].turbo == 2) {
	  /* two buttons pushed */
	  state.player[state.col2plr[1]].turbo = 1;
	} else
	  state.player[state.col2plr[1]].turbo = 0;
      }
    }
    if (state.player[state.col2plr[1]].inversed_controls != 0) {
      if (flag2)
	state.player[state.col2plr[1]].next_way ^= 2;
      state.player[state.col2plr[1]].tunnel_inverse = 1;
    } else
      state.player[state.col2plr[1]].tunnel_inverse = 0;

/*   if ((state.player[state.col2plr[1]].spec==t_tunnel)) printf("ss:%d,nxss:%d,ssold:%d,ssold2:%d,sstun:%d.\n", */
/*       state.player[state.col2plr[1]].way,state.player[state.col2plr[1]].next_way,state.player[state.col2plr[1]].old_way,state.player[state.col2plr[1]].old_old_way,state.player[state.col2plr[1]].tunnel_way); */
    if (((state.player[state.col2plr[1]].next_way ^ 2) == state.player[state.col2plr[1]].tunnel_way)
	&& (state.player[state.col2plr[1]].spec == t_tunnel))
      state.player[state.col2plr[1]].next_way = state.player[state.col2plr[1]].tunnel_way;
  }
  if (opt.ctrl_one ^ opt.ctrl_two) {
    if (is_joystick_button_a (1) && enable_blit)
      keyboard_map[HK_Pause] = 1;
  }
}

static unsigned char
play_game (char cont)
{
  int n, i;
  a_keycode t;
  char notbyebye = 1, flag;
  int l = 0, pos, u;
  char editflag = 0;
  static char tmpname[20];
  char bufstr[32];
  a_sprite *levelname;
  int exit_code;

  dmsg (D_SECTION, "-- play game --");

  in_menu = 0;
  tutor = (char) (state.game_mode == M_QUEST && current_quest_level == 0);
  if (loadulevel == 1) {
    char* tmp = get_non_null_rsc_file ("levels-dir");
    strappend (tmp, level_name);
    if (load_level (level_name, cont))
      emsg (_("Error during level loading"));
    free (tmp);
  } else if (state.game_mode == M_QUEST /*&& questmode==0 */ )
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
  }
  init_camera ();
  init_keyboard_map ();
  n = 1;

  update_htimers ();
  reset_htimer_with_offset (event_htimer, 4);
  reset_htimer (clock_htimer);
  reset_htimer (bonus_anim_htimer);
  reset_htimer (tiles_anim_htimer);
  reset_htimer (blink_htimer);
  state_start_game (&state);

  dmsg (D_SECTION, "introduce game");

  output_screen ((char) n);	/* update corner[] */
  process_input_events ();

  if (state.game_mode == M_QUEST)
    sprintf (bufstr, _("-LEVEL %d-"), current_quest_level);
  else
    sprintf (bufstr, _("-ROUND %d/%d-"),
	     rounds_nbr_values[opt.gamerounds] - rounds + 1,
	     rounds_nbr_values[opt.gamerounds]);

  levelname = compile_menu_text (bufstr, T_CENTERED, 99, 159);

  if (two_players == false) {
    int pendulum_pos;
    a_pendulum *p = pendulum_create ();
    n = 0;
    while (pendulum_update (p, &pendulum_pos) < 3000) {
      flip_buffer (pendulum_pos);

      DRAW_SPRITE (levelname, render_buffer[1]);

      flush_display (render_buffer[1]);
      output_screen ((char) n);
      process_input_events ();
      n = update_all (0);
    }
    pendulum_destroy (p);
  }
  if (two_players) {
    int buffer_pos = 39;

    corner[0] = render_buffer[0];
    clear_scr_area (corner[0]);
    DRAW_SPRITE (levelname, corner[0]);
    flush_display (corner[0]);

    sleep (1);
    update_htimers ();
    /* Restart the game, otherwise the sleep would cause a jump
       in the next frames.  */
    state_start_game (&state);

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
  state_start_players (&state);

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
    get_input_directions ();
    n = update_all (1);
    if (devparm && keyboard_map[HK_F12])
      state_level_set_exit_code (&state, 1);
    if (opt.ctrl_one ^ opt.ctrl_two)
      if (is_joystick_button_b (1) && enable_blit)
	notbyebye = quit_yes_no ();
    if (keyboard_map[HK_Escape] && enable_blit)
      notbyebye = quit_yes_no ();

    exit_code = state_level_exit_code (&state);
  } while (exit_code == 0 && notbyebye);

  dmsg (D_SECTION, "game finished");

  if (exit_code >= 1 && exit_code <= 4)
    state.player[exit_code - 1].wins++;

  radar_target_pos = 81;
  if (notbyebye) {

    dmsg (D_SECTION, "print end level info");

    uninit_keyboard_map ();
    l = 0;
    if (exit_code != 15)
      if (state.player[exit_code - 1].cpu == 2)
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
	a_pixel* tmp;

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
	if (t == HK_Down && state.game_mode == M_QUEST)
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
      if (l == 1 && (t == HK_Enter || flag == 0) && state.game_mode == M_QUEST) {
	event_sfx (67);
	load_save_records ();
	t = 0;
	l = 0;
	editflag = 0;
	do {
	  if (two_players == false) {
	    draw_saved_games_info (0, l, true);
	    flush_display (corner[0]);
	  } else {
	    a_pixel *tmp;

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
	      if (t == HK_BackSpace || t == HK_Delete) {
		saverec[l].used = 0;
		saverec[l].name[0] = 0;
		event_sfx (128);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
		write_save_one_record (l);
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
	      u = keycode_to_ascii (t);
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
		for (u = 0; u < 4; u++) {
		  saverec[l].points[u] = state.player[state.col2plr[u]].score;
		  copy_gameid (saverec[l].gid, game_id);
		  saverec[l].lifes[u] = state.player[state.col2plr[u]].lifes;
		}
		saverec[l].used = 1;
		editflag = 0;
		event_sfx (124);
		write_save_one_record (l);
		FREE_SPRITE0 (saverec_name[l]); /* force recompilation */
	      }
	    }
	  }
	} while ((t != HK_Escape && t != HK_Enter) || editflag != 0);
	/* while (keyboard_map[HK_Escape]) process_input_events (); */
	l = 1;
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

    if (l == 255 || state.game_mode == M_QUEST) {
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
	  a_pixel *tmp;

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
	a_pixel *tmp;
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
	  a_pixel *tmp;
	  tmp = corner[0];
	  draw_round_info (swapside ? -160 : 0);
	  corner[0] = corner[1];
	  draw_round_info (swapside ? 0 : -160);
	  corner[0] = tmp;
	  flush_display2 (corner[0], corner[1]);
	}
	if (! keyboard_map[HK_Enter])
	  flag = 0;
	output_screen ((char) n);
	process_input_events ();
	n = update_all (1);
	if (keyboard_map[HK_Escape])
	  if (quit_yes_no () == 0)
	    l = 255;
      } while ( /*! keyboard_map[HK_Escape] */ l != 255
	       && (flag || ! keyboard_map[HK_Enter]));
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
	  a_pixel *tmp;
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
	a_pixel *tmp;
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
  if ((!notbyebye) || (exit_code == 15))
    l = 255;
  uninit_keyboard_map ();
  unload_level ();
  nbr_tiles_cols = 15;
  in_menu = 1;
/* if (l!=0) cont=0; */

  return (l);
}

int
heroes_main (int argc, char *argv[])
{
  int i;

  mtrace (); /* GNU libc's malloc debugging facility */

  dmsg_init (argv[0]);
  dmsg (D_SECTION, "initialization");

  var_initialize ();		/* Needed by init_persona.  */
  init_persona ();

  hooks_core_initialize ();

  relocate_data (argv[0]);
  init_locales ();

  plugins_initialize ();

  init_sound_track_list ();

  /* Allow to override system-conf.  That's especially used by the
     testsuite; and not documented (although it could).  */
  {
    char *system_conf = getenv ("HEROES_SYSTEM_CONF");
    if (system_conf) {
      dmsg (D_SYSTEM, "HEROES_SYSTEM_CONF = %s", system_conf);
      set_rsc_file ("system-conf", system_conf, false);
    }
  }

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

  init_scores ();
  init_save_records ();
  user_persona_definitively ();

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

  /* Reopen the score and save file, because the user
     might have changed them from his configuration file.  */
  reinit_scores_if_needed ();
  reinit_save_records_if_needed ();

  freeze_sound_track_list ();

  dmsg (D_SYSTEM, "parsing command line");
  {
    int err = parse_argv (argc, argv, 0, 0);
    if (err < 0)
      exit (0);
    if (err > 0)
      exit (err);
  }

  dmsg (D_SYSTEM, "randomize");
  srand (time (0));

  read_level_list ();
  if (showlevels) {
    print_level_list ();
    exit (0);
  }

  browse_extra_directories ();
  if (reinitopt)
    reinit_preferences ();
  else
    load_preferences ();
  if (showprefs) {
    output_preferences (stdout);
    exit (0);
  }

  /* FIXME: remove this option.  */
  if (reinitsco) {
    clear_scores ();
    write_scores ();
  }

  if (reinitsav) {
    clear_save_records ();
    write_save_records ();
  }

  if (joyoff) {
    joystick_detected = 0;
    /* reset controlers configuration to keyboards */
    opt.ctrl_one = 0;
    opt.ctrl_two = 0;
  } else
    joyinit ();

  sys_persona ();
  if (init_sound_engine ())
    exit (2);
  user_persona ();

  /* We read the SFX configuration only once the sound engine has
     been initialized, because during this initialization we might
     decide to disable SFX (hence no need to load the config file).
     */
  if (read_sfx_conf ())
    emsg (_("error in sfx.cfg"));

  sys_persona ();
  init_video ();
  user_persona ();

  init_htimer ();
  clock_htimer = new_htimer (T_GLOBAL, HZ (10));
  blink_htimer = new_htimer (T_GLOBAL, HZ (6));
  event_htimer = new_htimer (T_GLOBAL, HZ (70));
  waving_htimer = new_htimer (T_GLOBAL, HZ (70));
  corner_htimer = new_htimer (T_GLOBAL, HZ (280));
  background_htimer = new_htimer (T_LOCAL, HZ (70));
  sound_track_htimer = new_htimer (T_GLOBAL, HZ (2));
  tiles_anim_htimer = new_htimer (T_GLOBAL, HZ (70));
  demo_trigger_htimer = new_htimer (T_GLOBAL, HZ (1));
  init_text_waving_step ();
  init_fader ();

  if (!directmenu || check_what == check_intro) {
    play_intro ();
    if (check_what == check_intro)
      exit_heroes (0);
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
  free_level_list ();
  uninit_bonuses ();
  close_buffers ();
  uninit_sound_engine ();
  uninit_video ();
  var_uninitialize ();
  uninit_sound_track_list ();

  plugins_finalize ();
  hooks_core_finalize ();

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
