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
#include "const.h"
#include "sfx.h"
#include "options.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "keysdef.h"
#include "heroes.h"
#include "structs.h"
#include "extras.h"
#include "display.h"
#include "savegame.h"
#include "hedlite.h"
#include "render.h"
#include "sound.h"
#include "menus.h"
#include "keyb.h"
#include "misc.h"
#include "rsc_files.h"
#include "bytesex.h"
#include "fader.h"
#include "scrtools.h"
#include "sprrle.h"
#include "sprtext.h"
#include "sprprogwav.h"
#include "sprshade.h"
#include "spropaque.h"
#include "sound.h"
#include "debugmsg.h"
#include "timer.h"
#include "joystick.h"
#include "pixelize.h"
#include "scores.h"
#include "helptext.h"
#include "readmake.h"
#include "endscroll.h"

static htimer_t lemming_htimer;
static sprite_t* left_arrow = 0;
static sprite_t* right_arrow = 0;
static sprite_t* checked_box[2] = {0, 0};
static sprite_t* cursor_bg = 0;
static sprite_t* cursor_fg = 0;
static sprite_t* horizontal_rule = 0;
static sprite_t* bigarr_cursor = 0; /* for the editor menu */
static sprite_t* sqr_cursor = 0; /* likewise */
static sprite_t* small_arrows[2][2] = {{ 0, 0 }, { 0, 0}};

static sprite_t* control_menu_txt = 0;
static sprite_t* sound_menu_txt = 0;
static sprite_t* music_vol_txt = 0;
static sprite_t* sfx_vol_txt = 0;
static sprite_t* screen_menu_txt = 0;
static sprite_t* game_menu_txt = 0;
static sprite_t* game_rounds_txt = 0;
static sprite_t* keyboard_menu_txt = 0;
static sprite_t* keyboard_keys_txt[12] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static sprite_t* extra_menu_txt = 0;
static sprite_t* extra_modes_txt[3] = { 0, 0, 0 };
static sprite_t* extra_combine_txt[3] = { 0, 0, 0 };
static sprite_t* credit_menu_txt = 0;
static sprite_t* pause_menu_txt = 0;
static sprite_t* quitgame_menu_txt = 0;
static sprite_t* quitheroes_menu_txt = 0;
static sprite_t* quit_yes_txt = 0;
static sprite_t* quit_no_txt = 0;
static sprite_t* ed_new_level_txt = 0;
static sprite_t* ed_existing_level_txt = 0;
static sprite_t* ed_name_txt = 0;
static sprite_t* ed_x_wrap_txt = 0;
static sprite_t* ed_y_wrap_txt = 0;
static sprite_t* ed_x_size_txt = 0;
static sprite_t* ed_y_size_txt = 0;
static sprite_t* ed_edit_txt = 0;
static sprite_t* edit_sel_txt = 0;
static sprite_t* edit_first_menu_txt = 0;
static sprite_t* edit_first_new_txt = 0;
static sprite_t* edit_first_load_txt = 0;
static sprite_t* playmenu_title_txt = 0;
static sprite_t* playmenu_players_txt[2] = { 0, 0 };
static sprite_t* playmenu_goback_txt = 0;
static sprite_t* playmenu_load_txt = 0;
static sprite_t* gamemode_txt[5] = { 0, 0, 0, 0, 0 };
static sprite_t* load_select_txt = 0;
static sprite_t* save_select_txt = 0;
sprite_t* saverec_name[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static sprite_t* enter_your_name_txt = 0;
static sprite_t* info_mode_quest_txt = 0;
static sprite_t* info_mode_deathm_txt = 0;
static sprite_t* info_mode_killem_txt = 0;
static sprite_t* info_mode_tcash_txt = 0;
static sprite_t* info_mode_color_txt = 0;
static sprite_t* info_mode_next_txt = 0;
static sprite_t* info_mode_save_txt = 0;
static sprite_t* info_mode_return_txt = 0;
static sprite_t* info_round_txt = 0;
static sprite_t* info_martian[4] = { 0, 0, 0, 0 };
static sprite_t* higher_scores_txt = 0;
static sprite_t* jukebox_frame = 0;
static sprite_t* jukebox_back = 0;
static sprite_t* jukebox_forw = 0;
static sprite_t* jukebox_quit = 0;
static sprite_t* vehicles_spr[4] = { 0, 0, 0, 0 };
static sprite_t* lemming[8] = { 0, 0, 0, 0, 0, 0, 0 };
static sprite_t* ctrl_ico[2] = { 0, 0 };
static sprite_t* autopilot_ico[2] = { 0, 0 };
static sprite_t* music_ico[7] = { 0, 0, 0, 0, 0, 0 };
static sprite_t* sfx_ico[7] = { 0, 0, 0, 0, 0, 0 };
static sprite_t* radar_ico[2] = { 0, 0 };
static sprite_t* infos_ico[2] = { 0, 0 };
static sprite_t* inertia_ico[2] = { 0, 0 };
static sprite_t* luminance_ico[7] = { 0, 0, 0, 0, 0, 0, 0 };
sprite_t* player_logo[4] = { 0, 0, 0, 0}; /* for end level info and game */
static sprite_t* player_ico[4] = { 0, 0, 0, 0}; /* for menus */
static sprite_t* speed_ico[5] = { 0, 0, 0, 0, 0};
static sprite_t* deck_digits[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static read_data_t* help_text = 0;

/* the following definitions are used to compile text-centered menus,
   that is, the main and the option menus */

typedef void (*entry_func_t) (void);

typedef struct {
  const char *name;
  entry_func_t func;
} menu_entry_t;

typedef struct {
  sprite_t *title;
  sprite_t **entries;
  entry_func_t *funcs;
  int lines;
  int *hrules;
  unsigned arrows_col;
  unsigned first_row;
} menu_t;

static void game_menu (void);
static void screen_menu (void);
static void sound_menu (void);
static void control_menu (void);
static void keyboard_menu (void);
static void extra_menu (void);

menu_entry_t options_entries[] = {
  { N_("GAME"),		game_menu },
  { N_("SCREEN"),	screen_menu },
  { N_("SOUND"),	sound_menu },
  { N_("CONTROL"),	control_menu },
  { N_("KEYS"),		keyboard_menu },
  { N_("EXTRAS"),	extra_menu },
  { N_("GO BACK"),	0 },
  { 0,			0 }
};

menu_t *option_menu_data;

menu_entry_t main_entries[] = {
  { N_("PLAY"),		0 },
  { N_("OPTIONS"),	0 },
  { N_("INFOS"),	0 },
  { N_("CREDITS"),	0 },
  { N_("SCORES"),	0 },
  { N_("EDITOR"),	0 },
  { N_("QUIT"),		0 },
  { 0,			0 }
};

menu_t *main_menu_data ;

static menu_t *
compile_menu (const char *name, const menu_entry_t* entries)
{
  int tlines;			/* number of text lines */
  int row;			/* row on screen */
  const menu_entry_t *pos;
  int i;
  unsigned max_width = 0;
  NEW (menu_t, menu);

  /* title */
  menu->title = compile_menu_text (name, T_CENTERED|T_WAVING, 12, 159);

  /* get the number of (text-)lines to draw */
  tlines = 0;
  for (pos = entries; pos->name; ++pos)
    ++tlines;
  menu->lines = tlines;

  /* allocate space for entries, rules, and functions */
  XMALLOC_ARRAY (menu->entries, tlines);
  XMALLOC_ARRAY (menu->funcs, tlines);
  XMALLOC_ARRAY (menu->hrules, tlines + 1);

  /* compute the row for the first text to draw
     (32 is the size reserved by the menu header) */
  row = 32 + (200 - 32) / 2 - (tlines * 20) / 2;
  menu->first_row = row;

  /* compile the entry texts */
  for (i = 0; i < tlines; ++i) {
    const char *ename = _(entries[i].name);
    unsigned width = compute_text_width (menu_font, ename, 0);
    menu->entries[i] = compile_menu_text (ename, T_CENTERED, row, 159);
    menu->funcs[i] = entries[i].func;
    if (width > max_width)
      max_width = width;
    row += 20;
    if (i + 1 != tlines)
      menu->hrules[i] = row - 7;
  }
  menu->arrows_col = (160 - max_width/2) - 20;

  return menu;
}

static void
free_menu (menu_t *menu)
{
  int i;
  i = menu->lines;
  while (i)
    free_sprite (menu->entries[--i]);
  free (menu->entries);
  free (menu->hrules);
  free_sprite (menu->title);
  free (menu->funcs);
  free (menu);
}

void
init_menus_sprites (void)
{
  pcx_image_t icons_img, jukebox_img;

  pcx_load_from_rsc ("menu-pictures-img", &icons_img);
  pcx_load_from_rsc ("jukebox-img", &jukebox_img);

  lemming_htimer = new_htimer (T_GLOBAL, HZ (18));

  left_arrow = compile_sprrle (IMGPOS (main_font_img, 50, 134), 0,
			       20, 13, main_font_img.width, xbuf);
  right_arrow = compile_sprrle (IMGPOS (main_font_img, 50, 121), 0,
				20, 13, main_font_img.width, xbuf);
  checked_box[0] = compile_sprrle (IMGPOS (main_font_img, 50, 218), 0,
				   14, 21, main_font_img.width, xbuf);
  checked_box[1] = compile_sprrle (IMGPOS (main_font_img, 50, 239), 0,
				   14, 21, main_font_img.width, xbuf);
  cursor_bg = compile_sprrle (IMGPOS (main_font_img, 53, 181), 0,
			      10, 37, main_font_img.width, xbuf);
  cursor_fg = compile_sprrle (IMGPOS (main_font_img, 51, 174), 0,
			      14, 7, main_font_img.width, xbuf);
  horizontal_rule = compile_sprrle (IMGPOS (main_font_img, 61, 0), 0,
				    3, 120, main_font_img.width, xbuf);
  bigarr_cursor = compile_spropaque (IMGPOS (main_font_img, 50, 260),
				     11, 16, main_font_img.width, xbuf);
  sqr_cursor = compile_spropaque (IMGPOS (main_font_img, 81, 32),
				  5, 8, main_font_img.width, xbuf);

  small_arrows[1][0] = compile_sprrle (IMGPOS (main_font_img, 60, 147), 0,
				       10, 10, main_font_img.width, xbuf);
  small_arrows[1][1] = compile_sprrle (IMGPOS (main_font_img, 60, 157), 0,
				       10, 10, main_font_img.width, xbuf);
  small_arrows[0][0] = compile_sprrle (IMGPOS (main_font_img, 50, 147), 0,
				       10, 10, main_font_img.width, xbuf);
  small_arrows[0][1] = compile_sprrle (IMGPOS (main_font_img, 50, 157), 0,
				       10, 10, main_font_img.width, xbuf);

  ctrl_ico[0] = compile_spropaque (IMGPOS (icons_img, 106, 0),
				   18, 32, icons_img.width, xbuf);
  ctrl_ico[1] = compile_spropaque (IMGPOS (icons_img, 106 + 19, 0),
				18, 32, icons_img.width, xbuf);
  autopilot_ico[0] = compile_spropaque (IMGPOS (icons_img, 11, 144),
					18, 32, icons_img.width, xbuf);
  autopilot_ico[1] = compile_spropaque (IMGPOS (icons_img, 11 + 19, 144),
					18, 32, icons_img.width, xbuf);
  {
    int i;
    for (i = 0; i < 7; ++i) {
      music_ico[i] = compile_spropaque (IMGPOS (icons_img, 68 + 19 * i, 36),
					18, 32, icons_img.width, xbuf);
      sfx_ico[i] = compile_spropaque (IMGPOS (icons_img, 68 + 19 * i, 72),
				      18, 32, icons_img.width, xbuf);
      luminance_ico[i] =
	compile_spropaque (IMGPOS (icons_img, 11 + 19 * i, 252),
			   18, 32, icons_img.width, xbuf);
    }
  }
  radar_ico[0] = compile_spropaque (IMGPOS (icons_img, 11, 36),
				    18, 32, icons_img.width, xbuf);
  radar_ico[1] = compile_spropaque (IMGPOS (icons_img, 11 + 19, 36),
				    18, 32, icons_img.width, xbuf);
  infos_ico[0] = compile_spropaque (IMGPOS (icons_img, 11, 180),
				    18, 32, icons_img.width, xbuf);
  infos_ico[1] = compile_spropaque (IMGPOS (icons_img, 11 + 19, 180),
				    18, 32, icons_img.width, xbuf);
  inertia_ico[0] = compile_spropaque (IMGPOS (icons_img, 68, 109),
				      18, 32, icons_img.width, xbuf);
  inertia_ico[1] = compile_spropaque (IMGPOS (icons_img, 68 + 19, 108),
				      18, 32, icons_img.width, xbuf);
  {
    int i;
    for (i = 0; i < 4; ++i) {
      player_logo[i] =
	compile_spropaque (IMGPOS (main_font_img, 72, 196 + i * 28),
			   11, 28, main_font_img.width, xbuf);
      player_ico[i] =
	compile_spropaque (IMGPOS (icons_img, 68 + 19 * i, 144),
			   18, 32, icons_img.width, xbuf);
    }
    for (i = 0; i < 5; ++i)
      speed_ico[i] =
	compile_spropaque (IMGPOS (icons_img, 11 + 19 * (4 - i), 216),
			   18, 32, icons_img.width, xbuf);
  }

  /* control menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("CONTROL OPTIONS"),
				   T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (_("PLAYER 1"),
				   T_FLUSHED_LEFT, 39, 56));
  add_sprprog0 (compile_menu_text (_("AUTO PILOT"),
				   T_FLUSHED_LEFT, 72, 56));
  add_sprprog0 (compile_menu_text (_("PLAYER 2"),
				   T_FLUSHED_LEFT, 111, 56));
  add_sprprog0 (compile_menu_text (_("AUTO PILOT"),
				   T_FLUSHED_LEFT, 144, 56));
  add_sprprog0 (compile_menu_text (_("GO BACK"),
				   T_FLUSHED_LEFT, 182, 56));
  control_menu_txt = end_sprprog ();

  /* sound menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("SOUND OPTIONS"),
				   T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (_("MUSIC"), T_FLUSHED_LEFT, 39, 56));
  add_sprprog0 (compile_menu_text (_("SFX"), T_FLUSHED_LEFT, 109, 56));
  add_sprprog0 (compile_menu_text (_("GO BACK"), T_FLUSHED_LEFT, 179, 56));
  sound_menu_txt = end_sprprog ();

  music_vol_txt = compile_menu_text (_("MUSIC VOL"), T_FLUSHED_LEFT, 74, 56);
  sfx_vol_txt = compile_menu_text (_("SFX VOL"), T_FLUSHED_LEFT, 144, 56);

  /* screen menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("SCREEN OPTIONS"),
				   T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (_("RADAR"), T_FLUSHED_LEFT, 41, 56));
  add_sprprog0 (compile_menu_text (_("INFOS"), T_FLUSHED_LEFT, 75, 56));
  add_sprprog0 (compile_menu_text (_("LIGHTING"), T_FLUSHED_LEFT, 109, 56));
  add_sprprog0 (compile_menu_text (_("INERTIA"), T_FLUSHED_LEFT, 143, 56));
  add_sprprog0 (compile_menu_text (_("GO BACK"), T_FLUSHED_LEFT, 179, 56));
  screen_menu_txt = end_sprprog ();

  /* game menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("GAME OPTIONS"),
				   T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (_("PLAYER 1"), T_FLUSHED_LEFT, 33, 56));
  add_sprprog0 (compile_menu_text (_("PLAYER 2"), T_FLUSHED_LEFT, 57, 56));
  add_sprprog0 (compile_menu_text (_("PLAYER 3"), T_FLUSHED_LEFT, 81, 56));
  add_sprprog0 (compile_menu_text (_("PLAYER 4"), T_FLUSHED_LEFT, 105, 56));
  add_sprprog0 (compile_menu_text (_("SPEED"), T_FLUSHED_LEFT, 129, 56));
  add_sprprog0 (compile_menu_text (_("GO BACK"), T_FLUSHED_LEFT, 177, 56));
  add_sprprog (compile_spropaque (IMGPOS (icons_img, 68 + 19 * 4, 144),
				  18, 32, icons_img.width, xbuf),
	       149 * xbuf + 20);
  game_menu_txt = end_sprprog ();

  /* keyboard menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("KEYBOARD DEFS"),
				   T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (_("GO BACK"), T_CENTERED, 188, 159));
  /* 1st player */
  add_sprprog0 (compile_menu_text (_("PLAYER 1"), T_CENTERED, 25, 159));
  add_sprprog0 (compile_menu_text (_("UP"), T_FLUSHED_LEFT, 38, 25));
  add_sprprog0 (compile_menu_text (_("LEFT"), T_FLUSHED_LEFT, 60, 25));
  add_sprprog0 (compile_menu_text (_("DOWN"), T_FLUSHED_LEFT, 49, 25));
  add_sprprog0 (compile_menu_text (_("RIGHT"), T_FLUSHED_LEFT, 71, 25));
  add_sprprog0 (compile_menu_text (_("TURBO"), T_FLUSHED_LEFT, 82, 25));
  add_sprprog0 (compile_menu_text (_("STOP"), T_FLUSHED_LEFT, 93, 25));
  /* 2nd player */
  add_sprprog0 (compile_menu_text (_("PLAYER 2"), T_CENTERED, 108, 159));
  add_sprprog0 (compile_menu_text (_("UP"), T_FLUSHED_LEFT, 121, 25));
  add_sprprog0 (compile_menu_text (_("LEFT"), T_FLUSHED_LEFT, 143, 25));
  add_sprprog0 (compile_menu_text (_("DOWN"), T_FLUSHED_LEFT, 132, 25));
  add_sprprog0 (compile_menu_text (_("RIGHT"), T_FLUSHED_LEFT, 154, 25));
  add_sprprog0 (compile_menu_text (_("TURBO"), T_FLUSHED_LEFT, 165, 25));
  add_sprprog0 (compile_menu_text (_("STOP"), T_FLUSHED_LEFT, 176, 25));
  keyboard_menu_txt = end_sprprog ();

  /* extra menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("EXTRAS LEVELS"),
				   T_CENTERED|T_WAVING, 5, 169));
  add_sprprog0 (compile_menu_text (_("GO BACK"), T_FLUSHED_LEFT, 180, 20));
  extra_menu_txt = end_sprprog ();
  extra_modes_txt[0] = compile_menu_text (_("ORIGINAL LEVELS"),
					  T_FLUSHED_LEFT, 35, 20);
  extra_modes_txt[1] = compile_menu_text (_("ORIGINAL+EXTRAS"),
					  T_FLUSHED_LEFT, 35, 20);
  extra_modes_txt[2] = compile_menu_text (_("EXTRAS ONLY"),
					  T_FLUSHED_LEFT, 35, 20);
  extra_combine_txt[0] = compile_menu_text (_("EXTRAS: NONE"),
					    T_FLUSHED_LEFT, 57, 20);
  extra_combine_txt[1] = compile_menu_text (_("EXTRAS: ALL"),
					    T_FLUSHED_LEFT, 57, 20);
  extra_combine_txt[2] = compile_menu_text (_("EXTRAS: SELECT"),
					    T_FLUSHED_LEFT, 57, 20);

  /* credit menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (_("CREDITS"),
				   T_CENTERED|T_WAVING, 10, 159));
  add_sprprog0 (compile_menu_text (_("GFX AND IDEA:"), T_FLUSHED_LEFT, 40, 1));
  add_sprprog0 (compile_menu_text ("a GUEN",
				   T_FLUSHED_RIGHT|T_WAVING, 40, 318));
  add_sprprog0 (compile_menu_text (_("MUSIC:"), T_FLUSHED_LEFT, 60, 1));
  add_sprprog0 (compile_menu_text ("b TNK",
				   T_FLUSHED_RIGHT|T_WAVING, 60, 318));
  add_sprprog0 (compile_menu_text ("c ALEXEL",
				   T_FLUSHED_RIGHT|T_WAVING, 72, 318));
  add_sprprog0 (compile_menu_text (_("CODE:"), T_FLUSHED_LEFT, 93, 1));
  add_sprprog0 (compile_menu_text ("b POLLUX",
				   T_FLUSHED_RIGHT|T_WAVING, 93, 318));
  add_sprprog0 (compile_menu_para
		(_("PLEASE READ THE LIST OF OTHER CONTRIBUTORS IN THE FILE 'THANKS'"),
		 T_CENTERED, 118, 159, 310));
  credit_menu_txt = end_sprprog ();

  jukebox_frame = compile_sprrle (IMGPOS (jukebox_img, 0, 0), 0,
				  19, 306, jukebox_img.width, xbuf);
  jukebox_forw = compile_sprrle (IMGPOS (jukebox_img, 19, 0), 0,
				 9, 12, jukebox_img.width, xbuf);
  jukebox_back = compile_sprrle (IMGPOS (jukebox_img, 19, 12), 0,
				 9, 12, jukebox_img.width, xbuf);
  jukebox_quit = compile_sprrle (IMGPOS (jukebox_img, 19, 24), 0,
				 9, 16, jukebox_img.width, xbuf);
  {
    int i;
    for (i = 0; i < 8; ++i)
      lemming[i] = compile_sprrle (IMGPOS (main_font_img, 81, 132 + 6 * i), 0,
				   10, 6, main_font_img.width, xbuf);
    for (i = 0; i < 10; ++i) {
      char text[2] = { 0, 0 };
      text[0] = '0' + i;
      deck_digits[i] = compile_deck_text (text, T_FLUSHED_LEFT, 186, 8 + 227);
    }
    deck_digits[10] = compile_deck_text (":", T_FLUSHED_LEFT, 186, 8 + 227);
  }

  /* pause menu */
  pause_menu_txt = compile_menu_text (_("PAUSE"), T_CENTERED|T_WAVING, 5, 159);

  /* quit y/n menus */

  quitgame_menu_txt = compile_menu_text (_("QUIT THIS GAME?"),
					 T_CENTERED|T_WAVING, 75, 159);
  quitheroes_menu_txt = compile_menu_text (_("REALLY QUIT?"),
					 T_CENTERED|T_WAVING, 75, 159);
  quit_no_txt = compile_menu_text (_("NO"), T_CENTERED, 110, 159);
  quit_yes_txt = compile_menu_text (_("YES"), T_CENTERED, 95, 159);

  /* editor menu */
  ed_new_level_txt = compile_menu_text (_("EXISTING LEVEL"),
					T_CENTERED|T_WAVING, 7, 159);
  ed_existing_level_txt = compile_menu_text (_("NEW LEVEL"),
					     T_CENTERED|T_WAVING, 7, 159);
  ed_name_txt = compile_menu_text (_("NAME"), T_FLUSHED_LEFT, 54, 8);
  ed_x_wrap_txt = compile_menu_text (_("X-WRAP"), T_FLUSHED_LEFT, 85, 8);
  ed_y_wrap_txt = compile_menu_text (_("Y-WRAP"), T_FLUSHED_LEFT, 106, 8);
  ed_x_size_txt = compile_menu_text (_("X-SIZE"), T_FLUSHED_LEFT, 137, 8);
  ed_y_size_txt = compile_menu_text (_("Y-SIZE"), T_FLUSHED_LEFT, 158, 8);
  ed_edit_txt = compile_menu_text (_("EDIT"), T_FLUSHED_LEFT, 186, 227);

  /* options menu */
  option_menu_data = compile_menu (_("OPTIONS"), options_entries);

  /* main menu */
  main_menu_data = compile_menu (_("HEROES"), main_entries);

  /* editor selector */
  edit_sel_txt = compile_menu_text (_("SELECT A LEVEL"),
				    T_CENTERED|T_WAVING, 10, 159);

  /* editor first menu */
  edit_first_menu_txt = compile_menu_text (_("EDITOR"), T_CENTERED|T_WAVING,
					   10, 159);
  edit_first_new_txt = compile_menu_text (_("NEW LEVEL"),
					  T_CENTERED, 105, 159);
  edit_first_load_txt = compile_menu_text (_("LOAD LEVEL"),
					   T_CENTERED, 85, 159);

  /* play menu */
  playmenu_title_txt = compile_menu_text (_("PLAY"), T_CENTERED|T_WAVING,
					  4, 159);
  playmenu_players_txt[0] = compile_menu_text (_("ONE PLAYER"),
					       T_CENTERED, 31, 159);
  playmenu_players_txt[1] = compile_menu_text (_("TWO PLAYER"),
					       T_CENTERED, 31, 159);
  playmenu_goback_txt = compile_menu_text (_("GO BACK"),
					   T_CENTERED, 187, 159);
  playmenu_load_txt = compile_menu_text (_("LOAD..."),
					 T_CENTERED, 160, 159);
  {
    int i;
    for (i = 0; i < 5; ++i)
      gamemode_txt[i] = compile_menu_text (_(mode_name[i]),
					   T_CENTERED, 0, 159);
  }
  /* saved game selection */
  save_select_txt = compile_menu_text (_("CHOSE A SAVE SLOT"),
				       T_CENTERED|T_WAVING, 10, 159);
  load_select_txt = compile_menu_text (_("CHOSE A LOAD SLOT"),
				       T_CENTERED|T_WAVING, 10, 159);

  /* enter your name */
  enter_your_name_txt = compile_menu_text (_("ENTER YOUR NAME:"),
					   T_CENTERED, 70, 159);

  /* end level info */
  info_mode_quest_txt = compile_menu_text (_("SIZE  PTS  LIVES"),
					   T_CENTERED, 50, 180);
  info_mode_deathm_txt = compile_menu_text (_("PTS  LIVES"),
					    T_CENTERED, 50, 221);
  info_mode_killem_txt = compile_menu_text (_(" MEN  PTS  LIVES"),
					    T_CENTERED, 50, 180);
  info_mode_tcash_txt = compile_menu_text (_("CA$H  PTS  LIVES"),
					   T_CENTERED, 50, 180);
  info_mode_color_txt = compile_menu_text (_("COLORS  PTS  LIVES"),
					   T_CENTERED, 50, 170);
  info_mode_next_txt = compile_menu_text (_("NEXT LEVEL"),
					  T_CENTERED, 150, 159);
  info_mode_save_txt = compile_menu_text (_("SAVE..."),
					  T_CENTERED, 170, 159);
  info_mode_return_txt = compile_menu_text (_("PRESS RETURN"),
					    T_CENTERED, 160, 159);
  {
    int i;
    for (i = 0; i < 4; ++i)
      info_martian[i] =
	compile_sprshade (IMGPOS (main_font_img, 120, 40 + i * 64),
			  0, 1, glenz[0], 19, 24, main_font_img.width, xbuf);
  }
  info_round_txt = compile_menu_text (_("WINS  PTS  LIVES"),
				      T_CENTERED, 50, 180);
  {
    int i;
    for (i = 0; i < 4; ++i)
      vehicles_spr[i] = compile_sprrle (IMGPOS (vehicles_img,
						0, (i << 6) + 16),
					0, 10, 12, vehicles_img.width, xbuf);
  }

  /* higher scores */
  higher_scores_txt = compile_menu_text (_("HIGHER SCORES"),
					 T_CENTERED|T_WAVING, 10, 159);
  img_free (&icons_img);
  img_free (&jukebox_img);

  help_text = compile_helptext ();
}

void
uninit_menus_sprites (void)
{
  free_htimer (lemming_htimer);
  FREE_SPRITE0 (left_arrow);
  FREE_SPRITE0 (right_arrow);
  FREE_SPRITE0 (checked_box[0]);
  FREE_SPRITE0 (checked_box[1]);
  FREE_SPRITE0 (cursor_bg);
  FREE_SPRITE0 (cursor_fg);
  FREE_SPRITE0 (horizontal_rule);
  FREE_SPRITE0 (bigarr_cursor);
  FREE_SPRITE0 (sqr_cursor);
  FREE_SPRITE0 (small_arrows[0][0]);
  FREE_SPRITE0 (small_arrows[0][1]);
  FREE_SPRITE0 (small_arrows[1][0]);
  FREE_SPRITE0 (small_arrows[1][1]);
  FREE_SPRITE0 (ctrl_ico[0]);
  FREE_SPRITE0 (ctrl_ico[1]);
  FREE_SPRITE0 (autopilot_ico[0]);
  FREE_SPRITE0 (autopilot_ico[1]);
  {
    int i;
    for (i = 0; i < 7; ++i) {
      FREE_SPRITE0 (music_ico[i]);
      FREE_SPRITE0 (sfx_ico[i]);
      FREE_SPRITE0 (luminance_ico[i]);
    }
  }
  FREE_SPRITE0 (radar_ico[0]);
  FREE_SPRITE0 (radar_ico[1]);
  FREE_SPRITE0 (infos_ico[0]);
  FREE_SPRITE0 (infos_ico[1]);
  FREE_SPRITE0 (inertia_ico[0]);
  FREE_SPRITE0 (inertia_ico[1]);
  {
    int i;
    for (i = 0; i < 4; ++i) {
      FREE_SPRITE0 (player_logo[i]);
      FREE_SPRITE0 (player_ico[i]);
    }
    for (i = 0; i < 5; ++i)
      FREE_SPRITE0 (speed_ico[i]);
  }

  FREE_SPRITE0 (control_menu_txt);
  FREE_SPRITE0 (sound_menu_txt);
  FREE_SPRITE0 (music_vol_txt);
  FREE_SPRITE0 (sfx_vol_txt);
  FREE_SPRITE0 (screen_menu_txt);
  FREE_SPRITE0 (game_menu_txt);
  FREE_SPRITE0 (game_rounds_txt);
  FREE_SPRITE0 (keyboard_menu_txt);
  {
    int i;
    for (i = 11; i >= 0; --i)
      FREE_SPRITE0 (keyboard_keys_txt[i]);
  }
  FREE_SPRITE0 (extra_menu_txt);
  {
    int i;
    for (i = 2; i >= 0; --i) {
      FREE_SPRITE0 (extra_combine_txt[i]);
      FREE_SPRITE0 (extra_modes_txt[i]);
    }
  }
  FREE_SPRITE0 (credit_menu_txt);
  FREE_SPRITE0 (jukebox_frame);
  FREE_SPRITE0 (jukebox_forw);
  FREE_SPRITE0 (jukebox_back);
  FREE_SPRITE0 (jukebox_quit);
  {
    int i;
    for (i = 0; i < 8; ++i)
      FREE_SPRITE0 (lemming[i]);
    for (i = 0; i < 11; ++i)
      FREE_SPRITE0 (deck_digits[i]);
  }
  FREE_SPRITE0 (pause_menu_txt);
  FREE_SPRITE0 (quitgame_menu_txt);
  FREE_SPRITE0 (quitheroes_menu_txt);
  FREE_SPRITE0 (quit_no_txt);
  FREE_SPRITE0 (quit_yes_txt);
  FREE_SPRITE0 (ed_new_level_txt);
  FREE_SPRITE0 (ed_existing_level_txt);
  FREE_SPRITE0 (ed_name_txt);
  FREE_SPRITE0 (ed_x_wrap_txt);
  FREE_SPRITE0 (ed_y_wrap_txt);
  FREE_SPRITE0 (ed_x_size_txt);
  FREE_SPRITE0 (ed_y_size_txt);
  FREE_SPRITE0 (ed_edit_txt);
  free_menu (option_menu_data);
  free_menu (main_menu_data);
  FREE_SPRITE0 (edit_sel_txt);
  FREE_SPRITE0 (edit_first_menu_txt);
  FREE_SPRITE0 (edit_first_new_txt);
  FREE_SPRITE0 (edit_first_load_txt);
  FREE_SPRITE0 (playmenu_title_txt);
  FREE_SPRITE0 (playmenu_players_txt[0]);
  FREE_SPRITE0 (playmenu_players_txt[1]);
  FREE_SPRITE0 (playmenu_goback_txt);
  FREE_SPRITE0 (playmenu_load_txt);
  {
    int i;
    for (i = 4; i >= 0; --i)
      FREE_SPRITE0 (gamemode_txt[i]);
  }
  FREE_SPRITE0 (load_select_txt);
  FREE_SPRITE0 (save_select_txt);
  {
    int i;
    for (i = 0; i < 10; ++i)
      FREE_SPRITE0 (saverec_name[i]);
  }
  FREE_SPRITE0 (enter_your_name_txt);
  FREE_SPRITE0 (info_mode_quest_txt);
  FREE_SPRITE0 (info_mode_deathm_txt);
  FREE_SPRITE0 (info_mode_killem_txt);
  FREE_SPRITE0 (info_mode_tcash_txt);
  FREE_SPRITE0 (info_mode_color_txt);
  FREE_SPRITE0 (info_mode_next_txt);
  FREE_SPRITE0 (info_mode_save_txt);
  FREE_SPRITE0 (info_mode_return_txt);
  {
    int i;
    for (i = 0; i < 4; ++i)
      FREE_SPRITE0 (info_martian[i]);
  }
  FREE_SPRITE0 (info_round_txt);
  {
    int i;
    for (i = 0; i < 4; ++i)
      FREE_SPRITE0 (vehicles_spr[i]);
  }
  FREE_SPRITE0 (higher_scores_txt);

  if (help_text)
    free_reader_data (help_text);
}

static void
arrows (unsigned int row, unsigned int col)
{
  DRAW_SPRITE (left_arrow, corner[0] + row * xbuf + col);
  DRAW_SPRITE (right_arrow, corner[0] + row * xbuf + 320 - col - 13);
}

void
waving_arrows (unsigned int row, unsigned int col)
{
  col += minisinus[read_htimer (waving_htimer) & 31];
  DRAW_SPRITE (left_arrow, corner[0] + row * xbuf + col);
  DRAW_SPRITE (right_arrow, corner[0] + row * xbuf + 320 - col - 13);
}

static void
chkbox (unsigned int row, unsigned int col, int checked)
{
  DRAW_SPRITE (checked_box[checked], corner[0] + row * xbuf + col);
}

static void
cursor (unsigned int row, unsigned int col,
	unsigned int value, unsigned int max)
{
  DRAW_SPRITE (cursor_bg, corner[0] + (row + 2) * xbuf + col);
  DRAW_SPRITE (cursor_fg, corner[0] + row * xbuf + col + 2 + (25*value/max));
}

static void
hrule (unsigned int row)
{
  DRAW_SPRITE (horizontal_rule, corner [0] + row * xbuf + 100);
}

static keycode_t
move_updown (keycode_t key, int *pos, int latest_pos)
{
  switch (key) {

  case HK_Up:
    if (*pos > 0)
      --*pos;
    else
      *pos = latest_pos;	/* wrap */
    break;

  case HK_Down:
    if (*pos < latest_pos)
      ++*pos;
    else
      *pos = 0;			/* wrap */
    break;

  case HK_Escape:
    if (*pos != latest_pos)	/* On first escape, */
      *pos = latest_pos;	/* go to the latest line; */
    else			/* on doubled espace, */
      key = HK_Enter;		/* arrange to escape the menu. */
    break;

  default:
    return key;			/* Unknown key, return it. */
  }

  /* The key has been handled.  Tell it to the user. */
  event_sfx (1);
  return key;
}

void
background_menu (void)
{
  static long int TTT = 0;

  TTT += read_htimer (background_htimer);

  camera_x[0] = 65536 * 24 * cos (TTT / 111.0);
  camera_y[0] = 65536 * 24 * sin (TTT / 175.0);
  camera_x[0] &= (map_info.xwrap << 16) | (0xffff);
  camera_y[0] &= (map_info.ywrap << 16) | (0xffff);
  inert_x[0] = camera_x[0];
  inert_y[0] = camera_y[0];
  compute_corner (0, 1);

  update_text_waving_step ();
  draw_level (0);
}

static void
display_menu (menu_t *menu, int l, bool blit)
{
  int line;
  background_menu ();
  draw_sprprogwav (menu->title, corner[0]);
  for (line = 0; line < menu->lines; ++line)
    draw_sprprogwav_if (l == line, menu->entries[line], corner[0]);
  for (line = 0; line < menu->lines - 1; ++line)
    hrule (menu->hrules[line]);
  waving_arrows (l * 20 + menu->first_row - 5, menu->arrows_col);
  if (blit)
    flush_display (corner[0]);
}

static void
exec_menu (menu_t *menu)
{
  int l = 0;
  int k;

  for (;;) {
    entry_func_t to_call;

    std_white_fadein (&tile_set_img.palette);
    do {
      display_menu (menu, l, true);
      if (key_or_joy_ready ()) {
	k = get_key_or_joy ();
	k = move_updown (k, &l, menu->lines - 1);
      } else
	k = 0;
    } while (k != HK_Enter);
    to_call = menu->funcs[l];

    if (to_call) {
      event_sfx (2);
      to_call ();
    } else {
      event_sfx (8);
      break;
    }
  }
}

static void
control_menu (void)
{
  int l = 0;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (ctrl_ico[opt.ctrl_one], corner[0] + 35 * xbuf + 20);
    DRAW_SPRITE (autopilot_ico[opt.autopilot_one], corner[0] + 68 * xbuf + 20);
    DRAW_SPRITE (ctrl_ico[opt.ctrl_two], corner[0] + 107 * xbuf + 20);
    DRAW_SPRITE (autopilot_ico[opt.autopilot_two], corner[0] + 140 * xbuf + 20);
    arrows (35 + l * 35 - 2 * (l == 1) + 2 * (l == 2 || l == 4), 1);
    chkbox (69, 260, opt.autopilot_one);
    chkbox (141, 260, opt.autopilot_two);
    hrule (95);
    DRAW_SPRITE (control_menu_txt, corner[0]);
    flush_display (corner[0]);
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 4);
      if (t == HK_Right || t == HK_Left || t == HK_Enter) {
	if (l == 0)
	  opt.ctrl_one ^= 1;
	if (l == 1)
	  opt.autopilot_one ^= 1;
	if (l == 2)
	  opt.ctrl_two ^= 1;
	if (l == 3)
	  opt.autopilot_two ^= 1;
	if (l < 4) {
	  if (l & 1)
	    event_sfx (3);
	  else
	    event_sfx (5);
	}
      }
    } else
      t = 0;
  } while (t != HK_Enter || l != 4);
  event_sfx (8);
}

static const char*
search_keyname (keycode_t key)
{
  const struct keynames_s* k = keynames;

  while (k->name && k->code != key)
    ++k;
  return k->name;
}

static void
keyboard_menu (void)
{
  int l = 0, testing = 0;
  int i;
  keycode_t t = 0;
  int unconfigured_keys = -1;
  /* line for each keyname */
  int keyline[12] = { 38, 49, 60, 71, 82, 93, 121, 132, 143, 154, 165, 176 };
  /* array used to change the order in which keys are printed. */
  int reorder[12] = { 0, 2, 1, 3, 4, 5, 6, 8, 7, 9, 10, 11 };

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();

    arrows (11 * l + 33 + 15 * (l >= 6) + 4 * (l == 12), 1);
    DRAW_SPRITE (keyboard_menu_txt, corner[0]);

    /* Draw the key name,
       generate the associated sprite if needed. */
    for (i = 0; i < 12; ++i) {
      int oi = reorder[i];
      if (l != i || testing == 0) {
	if (!keyboard_keys_txt[i]) { /* need to generate a sprite ? */
	  sprite_t *res;
	  int key = opt.player_keys[oi > 5][oi > 5 ? oi - 6 : oi];
	  const char *keyname = search_keyname (key);

	  if (!keyname) {	/* unknown key? print its code number */
	    char name[64];
	    sprintf (name, "(%d)", key);
	    res = compile_menu_text (name, T_FLUSHED_RIGHT,
				     keyline[i], 295);
	  } else
	    res = compile_menu_text (keyname, T_FLUSHED_RIGHT,
				     keyline[i], 295);
	  keyboard_keys_txt[i] = res;
	}
	DRAW_SPRITE (keyboard_keys_txt[i], corner[0]);
      }
    }
    flush_display (corner[0]);

    if (testing == 0) {
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
	t = move_updown (t, &l, 12);
	if (t == HK_Enter && l != 12) {
	  testing = 1;
	  event_sfx (6);
	}
      } else
	t = 0;
    } else if (testing == 1) {
      if (!key_or_joy_ready ()) {
	testing = 2;
      }
    } else if (testing == 2) {
      if (key_ready ()) {
	int ll = reorder[l];

	t = get_key ();

	for (i = 0; i != 6; i++) {
	  if (ll != i && opt.player_keys[0][i] == t)
	    opt.player_keys[0][i] = HK_NIL;
	  if (ll != i + 6 && opt.player_keys[1][i] == t)
	    opt.player_keys[1][i] = HK_NIL;
	}
	if (ll < 6)
	  opt.player_keys[0][ll] = t;
	else
	  opt.player_keys[1][ll - 6] = t;

	/* force the regeneration of key name on next display */
	FREE_SPRITE0 (keyboard_keys_txt[l]);
	keyboard_keys_txt[l] = 0;

	l++;

	testing = 0;
	event_sfx (7);
      }
    }
    unconfigured_keys = -1;
    for (i = 0; i < 12; ++i)
      if (i < 6) {
	if (opt.player_keys[0][i] == HK_NIL) {
	  unconfigured_keys = i;
	  break;
	}
      } else {
	if (opt.player_keys[1][i - 6] == HK_NIL) {
	  unconfigured_keys = i;
	  break;
	}
      }
    if (unconfigured_keys >= 0 && t == HK_Enter && l == 12) {
      l = unconfigured_keys;
      if (l == 1)
	l = 2;
      else if (l == 2)
	l = 1;
      else if (l == 7)
	l = 8;
      else if (l == 8)
	l = 7;
    }
  } while (t != HK_Enter || l != 12);
  event_sfx (8);
}

static void
sound_menu (void)
{
  int l = 0;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (music_ico[0], corner[0] + 35 * xbuf + 20);
    DRAW_SPRITE (sfx_ico[0], corner[0] + 105 * xbuf + 20);
    arrows (35 + l * 35, 1);
    chkbox (36, 260, opt.music);
    chkbox (106, 260, opt.sfx);
    if (opt.music) {
      DRAW_SPRITE (music_ico[opt.music_volume / 2], corner[0] + 70 * xbuf + 20);
      cursor (72, 251, 13 - opt.music_volume, 13);
      DRAW_SPRITE (music_vol_txt, corner[0]);
    }
    if (opt.sfx) {
      DRAW_SPRITE (sfx_ico[opt.sfx_volume / 2], corner[0] + 140 * xbuf + 20);
      cursor (142, 251, 13 - opt.sfx_volume, 13);
      DRAW_SPRITE (sfx_vol_txt, corner[0]);
    }
    DRAW_SPRITE (sound_menu_txt, corner[0]);
    flush_display (corner[0]);
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 4);
      /* If music or sfx are disabled, we can't be on position 1 or 3.
	 Therefore we call move_updown again to make a second step
	 in the same direction. */
      if ((!opt.music && l == 1) || (!opt.sfx && l == 3))
	t = move_updown (t, &l, 4);

      if (t == HK_Right || t == HK_Left || t == HK_Enter) {
	if (l != 4) {
	  if (l & 1)
	    event_sfx (4);
	  else
	    event_sfx (3);
	}
      }
      if (t == HK_Right) {
	if (l == 0)
	  opt.music ^= 1;
	if (l == 1 && opt.music_volume > 0)
	  opt.music_volume--;
	if (l == 2)
	  opt.sfx ^= 1;
	if (l == 3 && opt.sfx_volume > 0)
	  opt.sfx_volume--;
      }
      if (t == HK_Left) {
	if (l == 0)
	  opt.music ^= 1;
	if (l == 1 && opt.music_volume < 12)
	  opt.music_volume++;
	if (l == 2)
	  opt.sfx ^= 1;
	if (l == 3 && opt.sfx_volume < 12)
	  opt.sfx_volume++;
      }
      if (t == HK_Enter) {
	if (l == 0)
	  opt.music ^= 1;
	if (l == 1) {
	  if (opt.music_volume < 12)
	    opt.music_volume++;
	  else
	    opt.music_volume = 0;
	}
	if (l == 2)
	  opt.sfx ^= 1;
	if (l == 3) {
	  if (opt.sfx_volume < 12)
	    opt.sfx_volume++;
	  else
	    opt.sfx_volume = 0;
	}
      }
      set_volume ();
    } else
      t = 0;
  } while (t != HK_Enter || l != 4);
  event_sfx (8);
}

static void
screen_menu (void)
{
  int l = 0;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();

    DRAW_SPRITE (radar_ico[opt.radar_map], corner[0] + 37 * xbuf + 20);
    DRAW_SPRITE (infos_ico[opt.display_infos], corner[0] + 71 * xbuf + 20);
    DRAW_SPRITE (luminance_ico[opt.luminance], corner[0] + 105 * xbuf + 20);
    DRAW_SPRITE (inertia_ico[opt.inertia], corner[0] + 139 * xbuf + 20);

    arrows (37 + l * 34 + 2 * (l == 4), 1);
    chkbox (38, 260, opt.radar_map);
    chkbox (72, 260, opt.display_infos);

    cursor (108, 251, 6 - opt.luminance, 6);
    chkbox (140, 260, opt.inertia);
    DRAW_SPRITE (screen_menu_txt, corner[0]);

    flush_display (corner[0]);

    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 4);
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l < 5) {
	  if (l == 0)
	    opt.radar_map ^= 1;
	  else if (l == 1)
	    opt.display_infos ^= 1;
	  else if (l == 2) {
	    if (t == HK_Right && opt.luminance > 0)
	      --opt.luminance;
	    else if (t == HK_Left && opt.luminance < 6)
	      ++opt.luminance;
	    set_pal_with_luminance (&tile_set_img.palette);
	  } else if (l == 3)
	    opt.inertia ^= 1;

	  if (l == 4)
	    event_sfx (4);
	  else
	    event_sfx (3);
	}
    } else
      t = 0;
  } while (t != HK_Enter || l != 4);
  event_sfx (8);
}

static void
game_menu (void)
{
  int l = 0, tmp;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (player_ico[opt.player_color[0]], corner[0] + 29 * xbuf + 20);
    DRAW_SPRITE (player_ico[opt.player_color[1]], corner[0] + 53 * xbuf + 20);
    DRAW_SPRITE (player_ico[opt.player_color[2]], corner[0] + 77 * xbuf + 20);
    DRAW_SPRITE (player_ico[opt.player_color[3]], corner[0] + 101 * xbuf + 20);
    DRAW_SPRITE (speed_ico[opt.speed * 2], corner[0] + 125 * xbuf + 20);
    cursor (128, 251, opt.speed, 2);
    cursor (154, 251, opt.gamerounds, 15);
    arrows (29 + l * 24 + 24 * (l > 2), 1);
    DRAW_SPRITE (game_menu_txt, corner[0]);

    if (!game_rounds_txt) {
      char rounds_txt[32];
      /* FIXME: handle plural */
      sprintf (rounds_txt, _("%d ROUNDS"), rounds_nbr_values[opt.gamerounds]);
      game_rounds_txt = compile_menu_text (rounds_txt,
					   T_FLUSHED_LEFT, 153, 56);
    }
    DRAW_SPRITE (game_rounds_txt, corner[0]);

    flush_display (corner[0]);

    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 5);
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l != 6) {
	  if ((l == 3) || (l == 4))
	    event_sfx (4);
	  else
	    event_sfx (5);
	}
      if (t == HK_Right || t == HK_Enter) {
	if (l == 3) {
	  if (opt.speed < 2)
	    opt.speed++;
	}
	if (l == 4) {
	  if (opt.gamerounds < 15) {
	    opt.gamerounds++;
	    /* Force regeneration of game_rounds_txt before next draw */
	    FREE_SPRITE0 (game_rounds_txt);
	    game_rounds_txt = 0;
	  }
	}
	if (l == 0) {
	  tmp = opt.player_color[0];
	  opt.player_color[0] = opt.player_color[1];
	  opt.player_color[1] = opt.player_color[2];
	  opt.player_color[2] = opt.player_color[3];
	  opt.player_color[3] = tmp;
	}
	if (l == 1) {
	  tmp = opt.player_color[1];
	  opt.player_color[1] = opt.player_color[2];
	  opt.player_color[2] = opt.player_color[3];
	  opt.player_color[3] = tmp;
	}
	if (l == 2) {
	  tmp = opt.player_color[2];
	  opt.player_color[2] = opt.player_color[3];
	  opt.player_color[3] = tmp;
	}
      }
      if (t == HK_Left) {
	if (l == 3)
	  if (opt.speed > 0)
	    opt.speed--;
	if (l == 4)
	  if (opt.gamerounds > 0) {
	    opt.gamerounds--;
	    /* Force regeneration of game_rounds_txt before next draw */
	    FREE_SPRITE0 (game_rounds_txt);
	    game_rounds_txt = 0;
	  }
	if (l == 0) {
	  tmp = opt.player_color[3];
	  opt.player_color[3] = opt.player_color[2];
	  opt.player_color[2] = opt.player_color[1];
	  opt.player_color[1] = opt.player_color[0];
	  opt.player_color[0] = tmp;
	}
	if (l == 1) {
	  tmp = opt.player_color[3];
	  opt.player_color[3] = opt.player_color[2];
	  opt.player_color[2] = opt.player_color[1];
	  opt.player_color[1] = tmp;
	}
	if (l == 2) {
	  tmp = opt.player_color[3];
	  opt.player_color[3] = opt.player_color[2];
	  opt.player_color[2] = tmp;
	}
      }
    } else
      t = 0;
  } while (t != HK_Enter || l != 5);
  event_sfx (8);
}

static void
extra_menu (void)
{
  int l = 0;
  int t, i, ll = 0;
  /* We store only the sprites for the displayed level names, as the list
     can be big (hmmm... really?) */
  sprite_t *levelnames[7] = { 0, 0, 0, 0, 0, 0, 0 };

  if (extra_nbr <= 0)
    return;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (extra_menu_txt, corner[0]);
    DRAW_SPRITE (extra_modes_txt[opt.extras], corner[0]);


    if (opt.extras == 0)
      DRAW_SPRITE (extra_combine_txt[0], corner[0]);
    else
      DRAW_SPRITE (extra_combine_txt[(extrasel == 0)?1:2], corner[0]);

    if (extrasel && opt.extras != 0) {
      hrule (72);
      hrule (169);
      for (i = -3; i <= 3; i++)
	if ((i + ll) >= 0 && (unsigned int)(i + ll) < extra_nbr) {
	  if (!levelnames[3 + i]) {
	    char lname[FILENAME_SIZE + 1];
	    strcpy (lname, extra_list[i + ll].level_name);
	    levelnames[3 + i] = compile_menu_text (lname, T_FLUSHED_RIGHT,
						   118, 200);
	  }
	  draw_sprprogwav_if (i==0,
			      levelnames[3 + i], corner[0] + i * 13 * xbuf);
	  chkbox (115 + i * 13, 210, extra_selected_list[i + ll]);
	}
    }
    arrows (30 + l * 22 + 40 * (l == 2) + 78 * (l == 3), 1);

    flush_display (corner[0]);

    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (l == 2) {
	if (t == HK_Home || t == HK_End || t == HK_PageUp || t == HK_PageDown)
	  event_sfx (1);
	if (t == HK_Up) {
	  if (ll > 0) {
	    ll--;
	    /* Rotate the levelnames. */
	    FREE_SPRITE0 (levelnames[6]);
	    memmove (levelnames + 1, levelnames, 6 * sizeof (*levelnames));
	    levelnames[0] = 0;
	  } else
	    l = 1;
	}
	if (t == HK_Down) {
	  if ((unsigned int) (ll + 1) < extra_nbr) {
	    ll++;
	    /* Rotate the levelnames. */
	    FREE_SPRITE0 (levelnames[0]);
	    memmove (levelnames, levelnames + 1, 6 * sizeof (*levelnames));
	    levelnames[6] = 0;
	  } else
	    l = 3;
	}
	if (t == HK_Home || t == HK_End
	    || t == HK_PageUp || t == HK_PageDown) {
	  int j;
	  /* Free all levelnames. */
	  for (j = 6; j >= 0; --j) {
	    FREE_SPRITE0 (levelnames[j]);
	    levelnames[j] = 0;
	  }

	  if (t == HK_Home)
	    ll = 0;
	  else if (t == HK_End)
	    ll = extra_nbr - 1;
	  else if (t == HK_PageUp)
	    ll = (ll > 7) ? (ll - 7) : 0;
	  else /* t == HK_PageDown */
	    ll = (((unsigned int) (ll + 7) < extra_nbr) ?
		  (ll + 7) : (extra_nbr - 1));
	}
      } else {
	if (t == HK_Up) {
	  if (l > 0)
	    l--;
	  else
	    l = 3;
	  if (l == 2 && (extrasel == 0 || opt.extras == 0))
	    l = 1;
	}
	if (t == HK_Down) {
	  if (l < 3)
	    l++;
	  else
	    l = 0;
	  if (l == 2 && (extrasel == 0 || opt.extras == 0))
	    l = 3;
	}
      }
      if (t == HK_Escape) {
	if (l != 3)
	  l = 3;
	else
	  t = HK_Enter;
      }
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l != 3) {
	  if (l < 2)
	    event_sfx (5);
	  else
	    event_sfx (3);
	  if (l == 0)
	    switch (t) {
	    case HK_Enter:
	    case HK_Right:
	      opt.extras = ((opt.extras == 2) ? 0 : (opt.extras + 1));
	      break;
	    case HK_Left:
	      opt.extras = ((opt.extras == 0) ? 2 : (opt.extras - 1));
	      break;
	    }
	  if ((l == 1) && (opt.extras != 0))
	    extrasel ^= 1;
	  if (l == 2)
	    extra_selected_list[ll] ^= 1;
	}
    } else
      t = 0;
  } while (t != HK_Enter || l != 3);
  event_sfx (8);

  /* Free all levelnames. */
  for (i = 6; i >= 0; --i)
    FREE_SPRITE0 (levelnames[i]);
}

void
option_menu (void)
{
  exec_menu (option_menu_data);
}

void
draw_quit_menu (int l)
{
  DRAW_SPRITE (quitheroes_menu_txt, corner[0]);
  draw_sprprogwav_if (l == 0, quit_yes_txt, corner[0]);
  draw_sprprogwav_if (l == 1, quit_no_txt, corner[0]);
  waving_arrows (91 + l * 15, 90);
}

char
quit_menu (void)
{
  int l = 0;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    draw_quit_menu (l);
    flush_display (corner[0]);
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 1);
    } else
      t = 0;
  } while (t != HK_Enter);
  if (l == 1) {
    event_sfx (77);
    return (0);
  } else {
    event_sfx (78);
    return (1);
  }
}

void
draw_play_menu (int l)
{
  background_menu ();
  DRAW_SPRITE (playmenu_title_txt, corner[0]);
  hrule (21);
  draw_sprprogwav_if (l == 0, playmenu_players_txt[two_players], corner[0]);
  hrule (48);
  {
    int i;
    for (i = 0; i < 5; ++i)
      draw_sprprogwav_if (l == i + 1, gamemode_txt[i], corner[0]
			  + (65 + 16 * i - 5 * (i == 0)) * xbuf);
  }
  hrule (150);
  draw_sprprogwav_if (l == 6, playmenu_load_txt, corner[0]);
  hrule (177);
  draw_sprprogwav_if (l == 7, playmenu_goback_txt, corner[0]);
  waving_arrows (41 + l * 16 + 5 * (l > 1) - 15 * (l == 0) +
		 13 * (l == 6) + 23 * (l == 7), 5);
}

void
draw_main_menu (int l)
{
  display_menu (main_menu_data, l, false);
}

char tile_sets_names[10][3] =
  { "01", "02", "03", "04", "05", "06", "07", "08", "09", "10" };

static void
load_tile_set_preview (int num, pcx_image_t * ici)
{
  char *t = get_non_null_rsc_file ("editor-preview-prefix");
  t = strappend (t, tile_sets_names[num]);
  t = strappend (t, ".pcx");
  pcx_load (t, ici);
  free (t);
}

static void
editor_selector (void)
{
  int l = 0;
  int i = 0, t;
  sprite_t *filenames[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  if (extra_user_nbr == 1) {
    event_sfx (116);
    hmain (extra_list[0].level_name, 0, 0, 0, 0, 0);
    return;
  }
  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (edit_sel_txt, corner[0]);
    hrule (30);
    hrule (187);

    /* draw filenames (generate correspounding sprites if needed) */

    for (i = -5; i <= 5; i++)
      if ((i + l) >= 0 && (unsigned int) (i + l) < extra_user_nbr) {
	if (!filenames [i + 5])
	  filenames [i + 5] = compile_menu_text (extra_list[i + l].level_name,
						 T_CENTERED, 105, 159);
	draw_sprprogwav_if (i == 0, filenames [i + 5],
			    corner[0] + i * 13 * xbuf);
      }
    waving_arrows (101, 60);
    flush_display (corner[0]);
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape || t == HK_Home
	  || t == HK_End || t == HK_PageUp || t == HK_PageDown) {
	int j;
	event_sfx (1);
	/* free all filenames */
	for (j = 0; j < 11; ++j)
	  FREE_SPRITE0 (filenames[j]);
      }
      if (t == HK_Up) {
	if (l > 0)
	  l--;
	else
	  l = extra_user_nbr - 1;
      } else if (t == HK_Down) {
	if ((unsigned int) (l + 1) < extra_user_nbr)
	  l++;
	else
	  l = 0;
      } else if (t == HK_Home) {
	l = 0;
      } else if (t == HK_End) {
	l = extra_user_nbr - 1;
      } else if (t == HK_PageUp) {
	l = (l > 10) ? (l - 10) : 0;
      } else if (t == HK_PageDown)
	l = (((unsigned int) (l + 11) < extra_user_nbr)
	     ? (l + 10) : (extra_user_nbr - 1));
    } else
      t = 0;
  } while (t != HK_Enter && t != HK_Escape);
  if (t == HK_Enter) {
    event_sfx (116);
    hmain (extra_list[l].level_name, 0, 0, 0, 0, 0);
  } else
    event_sfx (8);
}

static void
editor_menu (void)
{
  pcx_image_t frmenu, tilesprev;
  int l = 0, t = 0, pos = 0, i;
  int xwrap = 15;
  int ywrap = 15;
  int xsize = 16;
  int ysize = 16;
  char l2, flag = 0, flaglock = 0;
  int tiles = 0;
  int j;
  FILE *tmphdl;
  level_header_t plinfo;
  char lname[FILENAME_SIZE + 1];
  char ssize[32];
  char titres[10][16] =
    { "CORRIDOR 1", "CAEROS", "THE DARK AGES", "CORRIDOR 2", "VOLCANO",
    "ELECTRIC DREAM", "METAL MASTER", "MOON 51", "CORRIDOR 3",
    "SWEET DREAM"
  };
  sprite_t *tileset_name_txt = 0;
  sprite_t *level_name_txt = 0;
  sprite_t *x_size_txt = 0;
  sprite_t *y_size_txt = 0;

  memset (lname,0,FILENAME_SIZE + 1);
  pcx_load_from_rsc ("new-level-menu-img", (pcx_image_t *) & frmenu);
  load_tile_set_preview (0, (pcx_image_t *) & tilesprev);

  corner[0] = render_buffer[0];
  for (i = 0; i < 52; i++)
    memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
	    tilesprev.buffer + i * 62, 62);

  std_white_fadein (&tilesprev.palette);
  do {
    /* generate mutable texts if needed */
    if (!tileset_name_txt)
      tileset_name_txt =
	compile_menu_text (titres[tiles], T_CENTERED, 33, 138);
    if (!level_name_txt)
      level_name_txt = compile_menu_text (lname, T_CENTERED, 54, 185);
    if (!x_size_txt) {
      sprintf (ssize, "%d", xsize);
      x_size_txt = compile_menu_text (ssize, T_CENTERED, 137, 248);
    }
    if (!y_size_txt) {
      sprintf (ssize, "%d", ysize);
      y_size_txt = compile_menu_text (ssize, T_CENTERED, 158, 248);
    }

    /* draw everything */
    update_text_waving_step ();
    j = minisinus[read_htimer (waving_htimer) & 31];

    copy_image_to_scr_area (&frmenu, corner[0]);
    chkbox (82, 170, (xwrap != -1));
    chkbox (103, 170, (ywrap != -1));
    cursor (135, 153, xsize - 15, 64 - 15);
    cursor (156, 153, ysize - 11, 64 - 11);

    if (l == 0)
      DRAW_SPRITE (bigarr_cursor, corner[0] + 271 + 32 * xbuf);
    else if (l == 1)
      DRAW_SPRITE (sqr_cursor, corner[0] + 275 + 56 * xbuf);
    else if (l == 2)
      DRAW_SPRITE (sqr_cursor, corner[0] + 194 + 87 * xbuf);
    else if (l == 3)
      DRAW_SPRITE (sqr_cursor, corner[0] + 194 + 108 * xbuf);
    else if (l == 4)
      DRAW_SPRITE (sqr_cursor, corner[0] + 194 + 139 * xbuf);
    else if (l == 5)
      DRAW_SPRITE (sqr_cursor, corner[0] + 194 + 160 * xbuf);
    if (l == 6) {
      DRAW_SPRITE (small_arrows[1][0], corner[0] + 214 + 186 * xbuf + j);
      DRAW_SPRITE (small_arrows[1][1], corner[0] + 299 + 186 * xbuf - j);
    } else {
      DRAW_SPRITE (small_arrows[0][0], corner[0] + 214 + 186 * xbuf);
      DRAW_SPRITE (small_arrows[0][1], corner[0] + 299 + 186 * xbuf);
    }
    if (flaglock)
      DRAW_SPRITE (ed_existing_level_txt, corner[0]);
    else
      DRAW_SPRITE (ed_new_level_txt, corner[0]);

    DRAW_SPRITE (tileset_name_txt, corner[0]);
    draw_sprprogwav_if (l == 1, ed_name_txt, corner[0]);
    DRAW_SPRITE (level_name_txt, corner[0]);
    draw_sprprogwav_if (l == 2, ed_x_wrap_txt, corner[0]);
    DRAW_SPRITE (x_size_txt, corner[0]);
    draw_sprprogwav_if (l == 3, ed_y_wrap_txt, corner[0]);
    DRAW_SPRITE (y_size_txt, corner[0]);
    draw_sprprogwav_if (l == 4, ed_x_size_txt, corner[0]);
    draw_sprprogwav_if (l == 5, ed_y_size_txt, corner[0]);
    draw_sprprogwav_if (l == 6, ed_edit_txt, corner[0]);

    flush_display (corner[0]);

    /* handle key events */
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up) {
	if (flaglock == 0)
	  if (l > 0)
	    l--;
	  else
	    l = 6;

	else if (l == 1)
	  l = 6;
	else
	  l = 1;
	event_sfx (110);
      } else if (t == HK_Down) {
	if (flaglock == 0)
	  if (l < 6)
	    l++;
	  else
	    l = 0;

	else if (l == 6)
	  l = 1;
	else
	  l = 6;
	event_sfx (110);
      } else if (l == 0 && (t == HK_Right || t == HK_Left)) {
	FREE_SPRITE0 (tileset_name_txt);
	if (t == HK_Right) {
	  if (tiles < 9)
	    tiles++;
	  else
	    tiles = 0;
	} else {
	  if (tiles > 0)
	    tiles--;
	  else
	    tiles = 9;
	}
	img_free ((pcx_image_t *) & tilesprev);
	load_tile_set_preview (tiles, (pcx_image_t *) & tilesprev);
	for (i = 0; i < 52; i++)
	  memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
		  tilesprev.buffer + i * 62, 62);
	set_pal_with_luminance (&tilesprev.palette);
	event_sfx (111);
      } else if (l == 1) {
	l2 = t & 255;
	if (l2 >= 'a' && l2 <= 'z')
	  l2 -= 'a' - 'A';
	if ((l2 >= '0' && l2 <= '9') || (l2 >= '@' && l2 <= 'Z')
	    || (l2 >= '#' && l2 <= '&') || l2 == '!' || l2 == '_') {
	  if (pos < 8) {
	    lname[pos] = l2;
	    pos++;
	    lname[pos] = 0;
	    event_sfx (112);
	    flag = 1;
	    FREE_SPRITE0 (level_name_txt);
	  }
	}
	if ((t == HK_BackSpace || t == HK_Delete) && (pos > 0)) {
	  pos--;
	  lname[pos] = 0;
	  event_sfx (113);
	  flag = 1;
	  FREE_SPRITE0 (level_name_txt);
	}
	if (flag) {
	  char *filename;

	  XMALLOC_ARRAY (filename, (strlen (levels_output_dir) + 1
				    + strlen (lname) + 5));
	  sprintf(filename, "%s/%s.lvl", levels_output_dir, lname);
	  tmphdl = fopen (filename, "rb");
	  if (tmphdl != NULL) {
	    if (fread ((void *) &plinfo, sizeof (level_header_t), 1, tmphdl)
		== 1) {
	      xsize = BSWAP32 (plinfo.xt);
	      ysize = BSWAP32 (plinfo.yt);
	      xwrap = BSWAP32 (plinfo.xwrap);
	      ywrap = BSWAP32 (plinfo.ywrap);
	      tiles = atol ((char *) &(plinfo.tile_set_name[5])) - 1;
	      img_free ((pcx_image_t *) & tilesprev);
	      load_tile_set_preview (tiles, (pcx_image_t *) & tilesprev);
	      for (i = 0; i < 52; i++)
		memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
			tilesprev.buffer + i * 62, 62);
	      set_pal_with_luminance (&tilesprev.palette);
	      flaglock = 1;
	      event_sfx (117);
	    } else
	      flaglock = 0;
	    fclose (tmphdl);
	    free (filename);
	  } else
	    flaglock = 0;
	  flag = 0;
	}
      } else if (l == 2 && (t == HK_Right || t == HK_Left || t == HK_Enter)) {
	event_sfx (114);
	if (xwrap != -1)
	  xwrap = -1;

	else {
	  if (xsize < 24) {
	    xsize = 16;
	    xwrap = 15;
	  } else if (xsize < 48) {
	    xsize = 32;
	    xwrap = 31;
	  } else {
	    xsize = 64;
	    xwrap = 63;
	  }
	  FREE_SPRITE0 (x_size_txt);
	}
      } else if (l == 3 && (t == HK_Right || t == HK_Left || t == HK_Enter)) {
	event_sfx (114);
	if (ywrap != -1)
	  ywrap = -1;

	else {
	  if (ysize < 24) {
	    ysize = 16;
	    ywrap = 15;
	  } else if (ysize < 48) {
	    ysize = 32;
	    ywrap = 31;
	  } else {
	    ysize = 64;
	    ywrap = 63;
	  }
	  FREE_SPRITE0 (y_size_txt);
	}
      } else if (l == 4 && (t == HK_Right || t == HK_Left || t == HK_Enter)) {
	event_sfx (115);
	if (t == HK_Left) {
	  if (xsize > 15)
	    xsize--;
	  else
	    xsize = 64;
	  xwrap = -1;
	}
	if (t == HK_Right || t == HK_Enter) {
	  if (xsize < 64)
	    xsize++;
	  else
	    xsize = 15;
	  xwrap = -1;
	}
	FREE_SPRITE0 (x_size_txt);
      } else if (l == 5 && (t == HK_Right || t == HK_Left || t == HK_Enter)) {
	event_sfx (115);
	if (t == HK_Left) {
	  if (ysize > 11)
	    ysize--;
	  else
	    ysize = 64;
	  ywrap = -1;
	}
	if (t == HK_Right || t == HK_Enter) {
	  if (ysize < 64)
	    ysize++;
	  else
	    ysize = 11;
	  ywrap = -1;
	}
	FREE_SPRITE0 (y_size_txt);
      }
    } else
      t = 0;
    if (l == 6 && t == HK_Enter && lname[0] == 0)
      l = 1;
  } while (t != HK_Escape && !(l == 6 && t == HK_Enter));
  img_free (&frmenu);
  img_free (&tilesprev);
  FREE_SPRITE0 (level_name_txt);
  FREE_SPRITE0 (tileset_name_txt);
  FREE_SPRITE0 (x_size_txt);
  FREE_SPRITE0 (y_size_txt);
  if (l == 6 && t == HK_Enter) {
    event_sfx (116);
    hmain (lname, tile_sets_names[tiles], xsize, ysize, xwrap, ywrap);

    /* update extra-levels list */
    free_extra_list ();
    browse_extra_directories ();
  }
  reset_htimer (background_htimer);
  std_white_fadein (&tile_set_img.palette);
}

void
editor_first_menu (void)
{
  int l = 0;
  int t;
  if (extra_user_nbr == 0) {
    editor_menu ();
    return;
  }

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    DRAW_SPRITE (edit_first_menu_txt, corner[0]);
    draw_sprprogwav_if (l == 0, edit_first_load_txt, corner[0]);
    draw_sprprogwav_if (l == 1, edit_first_new_txt, corner[0]);
    waving_arrows (81 + l * 20, 50);
    flush_display (corner[0]);
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up || t == HK_Down)
	l ^= 1;
    } else
      t = 0;
  } while (t != HK_Enter && t != HK_Escape);
  if (t == HK_Enter) {
    if (l == 0) {
      event_sfx (2);
      editor_selector ();
    } else if (l == 1) {
      event_sfx (2);
      editor_menu ();
    }
  } else
    event_sfx (8);
}

void
draw_saved_games_info (int decal, int l, bool save)
{
  int i;
  char c;

  if (save)
    DRAW_SPRITE (save_select_txt, corner[0] + decal);
  else
    DRAW_SPRITE (load_select_txt, corner[0] + decal);

  for (i = 0; i < 10; i++) {
    if (saverec[i].used != 0) {
      if ((in_menu == 0) && (saverec[i].magic == game_magic))
	c = 5;
      else
	c = 3;
    } else
      c = 0;
    draw_glenz_box (corner[0] + decal + (40 + i * 14) * xbuf + 2 * xbuf, c,
		    320, 6);
  }
  for (i = 0; i < 10; i++) {
    if (!saverec_name[i])
      saverec_name[i] = compile_menu_text (saverec[i].name,
					   T_CENTERED, 40 + i * 14, 159);
    DRAW_SPRITE (saverec_name[i], corner[0] + decal);
  }

  DRAW_SPRITE (left_arrow, corner[0] + decal + (35 + l * 14) * xbuf + 1);
  DRAW_SPRITE (right_arrow,
	       corner[0] + decal + (35 + l * 14) * xbuf + 320 - 1 - 13);
}

static void
jukebox_draw (int pos)
{
  int t, t2, dp;

  dp = read_htimer (sound_track_htimer);
  t = dp/2;
  dp &= 1;
  if (t > 5999)
    t = 5999;

  DRAW_SPRITE (jukebox_frame, corner[0] + 180 * xbuf + 8);
  if (pos == 0)
    DRAW_SPRITE (jukebox_forw, corner[0] + 184 * xbuf + 8 + 5);
  else if (pos == 1)
    DRAW_SPRITE (jukebox_back, corner[0] + 184 * xbuf + 8 + 27);
  else if (pos == 2)
    DRAW_SPRITE (jukebox_quit, corner[0] + 184 * xbuf + 8 + 274);

  if (soundtrack_title) {
    if (!soundtrack_title_sprite)
      soundtrack_title_sprite = compile_deck_text (soundtrack_title,
						   T_CENTERED, 186, 110);
    DRAW_SPRITE (soundtrack_title_sprite, corner[0]);
  }
  if (soundtrack_author) {
    if (!soundtrack_author_sprite)
      soundtrack_author_sprite = compile_deck_text (soundtrack_author,
						    T_CENTERED, 186, 197);
    DRAW_SPRITE (soundtrack_author_sprite, corner[0]);
  }

  t2 = t % 60;
  t /= 60;
  DRAW_SPRITE (deck_digits[t2 % 10], corner[0] + 20);
  DRAW_SPRITE (deck_digits[t2 / 10], corner[0] + 14);
  if (dp == 0)
    DRAW_SPRITE (deck_digits[10], corner[0] + 12);
  DRAW_SPRITE (deck_digits[t % 10], corner[0] + 6);
  DRAW_SPRITE (deck_digits[t / 10], corner[0]);
  {
    int lempos = read_htimer (lemming_htimer);
    DRAW_SPRITE (lemming[lempos & 7],
		 corner[0] + 190 * xbuf + (lempos / 2) - 6);
    if ((lempos / 2) >= 332)
      reset_htimer (lemming_htimer);
  }

  flush_display (corner[0]);
}

static int
jukebox_keys (int *pos)
{
  keycode_t k;

  if (!key_or_joy_ready ())
    return 1;

  k = get_key_or_joy ();
  if (k == HK_Up || k == HK_Down || k == HK_Left || k == HK_Right)
    event_sfx (79);
  if (k == HK_Up || k == HK_Left) {
    if (*pos > 0)
      --*pos;
    else
      *pos = 2;
  } else if (k == HK_Down || k == HK_Right) {
    if (*pos < 2)
      ++*pos;
    else
      *pos = 0;
  } else if (k == HK_Enter) {
    if (*pos == 2)
      k = HK_Escape;
    else {
      unload_soundtrack ();
      if (*pos == 0) {
	event_sfx (74);
	load_next_soundtrack ();
      }
      if (*pos == 1) {
	event_sfx (75);
	load_prev_soundtrack ();
      }
      play_soundtrack ();
      reset_htimer (sound_track_htimer);
    }
  }
  return (k != HK_Escape);
}

void
jukebox_menu (void)
{
  signed char sh;		/* shift */
  int l = 0;

  in_jokebox = 1;
  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();

    sh = minisinus[read_htimer (waving_htimer) & 31];
    draw_glenz_box (corner[0] + (42 + sh) * xbuf + 234, 2, 86, 6);
    draw_glenz_box (corner[0] + (62 + sh) * xbuf + 244, 3, 76, 6);
    draw_glenz_box (corner[0] + (74 + sh) * xbuf + 194, 4, 126, 6);
    draw_glenz_box (corner[0] + (95 + sh) * xbuf + 194, 5, 126, 6);
    DRAW_SPRITE (credit_menu_txt, corner[0]);
    hrule (28);
    hrule (109);
    hrule (171);
    jukebox_draw (l);
  } while (jukebox_keys (&l));

  event_sfx (76);
  in_jokebox = 0;
}

void
pause_menu (void)
{
  int l = 0;
  htimer_t pause_htimer;

  dmsg (D_SECTION, "pause menu");

  pause_htimer = new_htimer (T_GLOBAL, 1);

  halve_volume ();
  event_sfx (58);

  corner[0] = render_buffer[0];

  uninit_keyboard_map ();
  std_white_fadein (&dummy_background_img.palette);
  do {
    dummy_moving_background_render ();
    update_text_waving_step ();
    DRAW_SPRITE (pause_menu_txt, corner[0]);
    jukebox_draw (l);
  } while (jukebox_keys (&l));

  init_keyboard_map ();
  set_volume ();
  enable_blit = 0;
  event_sfx (59);

  /* delay important timers that continued running during the pause */
  shift_htimer (update_htimer, pause_htimer);
  shift_htimer (event_htimer, pause_htimer);

  free_htimer (pause_htimer);
  dmsg (D_SECTION, "exit pause menu");
  set_pal (tile_set_img.palette.global, 0, 768);
}

char
quit_yes_no (void)
{
  int l = 0;
  keycode_t t;
  htimer_t pause_htimer;

  dmsg (D_SECTION, "quit y/n menu");

  pause_htimer = new_htimer (T_GLOBAL, 1);

  corner[0] = render_buffer[0];

  if (opt.ctrl_one || opt.ctrl_two)
    do {
      get_joystick_state ();
      flush_display (corner[0]);
    } while (joystick_b[0] || joystick_b[1]);

  uninit_keyboard_map ();
  halve_volume ();
  event_sfx (85);
  std_white_fadein (&dummy_background_img.palette);
  do {
    dummy_moving_background_render ();
    update_text_waving_step ();

    DRAW_SPRITE (quitgame_menu_txt, corner[0]);
    draw_sprprogwav_if (l == 0, quit_yes_txt, corner[0]);
    draw_sprprogwav_if (l == 1, quit_no_txt, corner[0]);
    waving_arrows (91 + 15 * l, 90);

    flush_display (corner[0]);

    t = 0;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 1);
    } else
      t = 0;
  } while (t != HK_Enter);
  set_volume ();
  enable_blit = 0;
  if (l == 0)
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
  set_pal (tile_set_img.palette.global, 0, 768);
  return l;
}

void
enter_your_name (char c, char* name)
{
  keycode_t t = 0;
  int pos = 0;
  char l;
  char head[256];
  htimer_t pixelize_timer = new_htimer (T_GLOBAL, HZ (7));
  sprite_t *player_number;
  sprite_t *player_name = 0;
  pixel_t *pixbuf;

  XMALLOC_ARRAY (pixbuf, 200 * xbuf);

  sprintf (head, _("PLAYER %d, YOU CAME IN THE TOP 10"), c);
  player_number = compile_menu_para (head, T_CENTERED, 20, 159, 310);

  memset (name, 0, PLAYER_NAME_SIZE + 1);

  std_white_fadein (&tile_set_img.palette);
  event_sfx (73);
  do {
    background_menu ();
    DRAW_SPRITE (player_number, corner[0]);
    DRAW_SPRITE (enter_your_name_txt, corner[0]);
    if (!player_name)
      player_name = compile_menu_text (name, T_CENTERED, 120, 159);
    DRAW_SPRITE (player_name, corner[0]);
    hrule (112);
    hrule (135);
    {
      long p = read_htimer (pixelize_timer);
      if (p <= 6) {
	pixelize[6 - p] (pixbuf, corner[0]);
	flush_display (pixbuf);
      } else {
	flush_display (corner[0]);
      }
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
	  FREE_SPRITE0 (player_name); /* force recompilation */
	}
      if ((t == HK_BackSpace || t == HK_Delete) && (pos > 0)) {
	pos--;
	name[pos] = 0;
	event_sfx (71);
	FREE_SPRITE0 (player_name); /* force recompilation */
      }
    }
  } while (t != HK_Escape && t != HK_Enter);
  event_sfx (72);
  if (pos == 0 || t == HK_Escape)
    memset (name, 0, PLAYER_NAME_SIZE);
  free (pixbuf);
  free_htimer (pixelize_timer);
  FREE_SPRITE0 (player_number);
  FREE_SPRITE0 (player_name);
}

void
draw_end_level_info (int decal, char l)
{
  int i;
  char winner[128];
  char nbr[32];
  sprite_t *winner_txt = 0;
  sprite_t *lines[4][3] = { { 0, 0, 0 },
			    { 0, 0, 0 },
			    { 0, 0, 0 },
			    { 0, 0, 0 } };

  if (!winner_txt) {
    if (level_is_finished != 15) {
      sprintf (winner, _("PLAYER %d WON!"),
	       plr2col[level_is_finished - 1] + 1);
      draw_glenz_box (corner[0] + decal + 22 * xbuf,
		      level_is_finished + 1, 320, 6);
    } else {
      if (two_players)
	sprintf (winner, _("EVERYONE LOST!"));
      else
	sprintf (winner, _("YOU LOST!"));
      draw_glenz_box (corner[0] + decal + 22 * xbuf, 7, 320, 6);
    }
    winner_txt = compile_menu_text (winner, T_CENTERED|T_WAVING, 20, 159 + 20);
  }
  DRAW_SPRITE (winner_txt, corner[0] + decal);

  if (game_mode == M_QUEST)
    DRAW_SPRITE (info_mode_quest_txt, corner[0] + decal);
  else if (game_mode == M_DEATHM)
    DRAW_SPRITE (info_mode_deathm_txt, corner[0] + decal);
  else if (game_mode == M_KILLEM)
    DRAW_SPRITE (info_mode_killem_txt, corner[0] + decal);
  else if (game_mode == M_TCASH)
    DRAW_SPRITE (info_mode_tcash_txt, corner[0] + decal);
  else if (game_mode == M_COLOR)
    DRAW_SPRITE (info_mode_color_txt, corner[0] + decal);

  if ((level_is_finished != 15) && (game_mode == M_QUEST)) {
    draw_sprprogwav_if (l == 0, info_mode_next_txt, corner[0] + decal);
    draw_sprprogwav_if (l == 1, info_mode_save_txt, corner[0] + decal);
    waving_arrows (145 + l * 20, 45);
  } else {
    DRAW_SPRITE (info_mode_return_txt, corner[0] + decal);
  }
  for (i = 0; i < 4; i++) {
    draw_glenz_box (corner[0] + decal + (75 + i * 12) * xbuf +
		    2 * xbuf, col2plr[i] + 2, 284 + i * 6, 6);
    DRAW_SPRITE (vehicles_spr[col2plr[i]],
		 corner[0] + decal + (75 + i * 12) * xbuf + 284 + i * 6);
    DRAW_SPRITE (player_logo[col2plr[i]],
		 corner[0] + decal + (75 + i * 12) * xbuf + 5);
    if (player[col2plr[i]].martians_nbr)
      DRAW_SPRITE (info_martian[i],
		   corner[0] + decal + (69 + i * 12) * xbuf + 35);
    if (!lines[i][0]) {
      if (game_mode == M_QUEST)
	sprintf (nbr, "%d", (trail_size[col2plr[i]] + 1) / 5 - 1);
      else if (game_mode == M_DEATHM)
	sprintf (nbr, "   ");
      else if (game_mode == M_KILLEM)
	sprintf (nbr, "%d", player[col2plr[i]].lemmings_nbr);
      else if (game_mode >= M_TCASH)
	sprintf (nbr, "%d", player[col2plr[i]].cash);
      lines[i][0] = compile_menu_text (nbr, T_CENTERED, 75 + i * 12, 108 - 10);
    }
    DRAW_SPRITE (lines[i][0], corner[0] + decal);
    if (!lines[i][1]) {
      sprintf (nbr, "%d", player[col2plr[i]].score);
      lines[i][1] = compile_menu_text (nbr, T_CENTERED, 75 + i * 12, 182 - 10);
    }
    DRAW_SPRITE (lines[i][1], corner[0] + decal);
    if (!lines[i][2]) {
      sprintf (nbr, "%d", player[col2plr[i]].lifes);
      lines[i][2] = compile_menu_text (nbr, T_CENTERED, 75 + i * 12, 265 - 10);
    }
    DRAW_SPRITE (lines[i][2], corner[0] + decal);
  }
  FREE_SPRITE0 (winner_txt);
  for (i = 0; i < 4; ++i) {
    FREE_SPRITE0 (lines[i][0]);
    FREE_SPRITE0 (lines[i][1]);
    FREE_SPRITE0 (lines[i][2]);
  }
}

void
draw_round_info (int decal)
{
  int i;
  char info[128];
  sprite_t *winner_txt = 0;
  sprite_t *lines[4][3] = { { 0, 0, 0 },
			    { 0, 0, 0 },
			    { 0, 0, 0 },
			    { 0, 0, 0 } };

  draw_glenz_box (corner[0] + decal + 22 * xbuf, 1, 320, 6);
  if (!winner_txt) {
    sprintf (info, _("ROUND %d/%d"),
	     rounds_nbr_values[opt.gamerounds] - rounds + 1,
	     rounds_nbr_values[opt.gamerounds]);
    winner_txt = compile_menu_text (info, T_CENTERED, 20, 159);
  }
  DRAW_SPRITE (winner_txt, corner[0] + decal);
  DRAW_SPRITE (info_round_txt, corner[0] + decal);
  DRAW_SPRITE (info_mode_return_txt, corner[0] + decal);

  for (i = 0; i < 4; i++) {
    draw_glenz_box (corner[0] + decal + (75 + i * 12) * xbuf +
		    2 * xbuf, col2plr[i] + 2, 284 + i * 6, 6);
    DRAW_SPRITE (vehicles_spr[col2plr[i]],
		 corner[0] + decal + (75 + i * 12) * xbuf + 284 + i * 6);
    DRAW_SPRITE (player_logo[col2plr[i]],
		 corner[0] + decal + (75 + i * 12) * xbuf + 5);
    if (!lines[i][0]) {
      sprintf (info, "%d", player[col2plr[i]].wins);
      lines[i][0] = compile_menu_text (info,
				       T_CENTERED, 75 + i * 12, 108 - 10);
    }
    DRAW_SPRITE (lines[i][0], corner[0] + decal);
    if (!lines[i][1]) {
      sprintf (info, "%d", player[col2plr[i]].score);
      lines[i][1] = compile_menu_text (info,
				       T_CENTERED, 75 + i * 12, 182 - 10);
    }
    DRAW_SPRITE (lines[i][1], corner[0] + decal);
    if (!lines[i][2]) {
      sprintf (info, "%d", player[col2plr[i]].lifes);
      lines[i][2] = compile_menu_text (info,
				       T_CENTERED, 75 + i * 12, 265 - 10);
    }
    DRAW_SPRITE (lines[i][2], corner[0] + decal);
  }
  for (i = 0; i < 4; ++i) {
    FREE_SPRITE0 (lines[i][0]);
    FREE_SPRITE0 (lines[i][1]);
    FREE_SPRITE0 (lines[i][2]);
  }
}

void
scores_menu (void)
{
  keycode_t t;
  int i, j;
  char flag = 0;
  int rolldec = 0;
  signed char rollflag = 0;
  sprite_t *points_txt[5][10];
  sprite_t *highs_txt[5][10];

  for (i = 0; i < 5; ++i)
    for (j = 0; j < 10; ++j) {
      char points[32];
      highs_txt[i][j] = compile_menu_text (highs[i][j].name,
					   T_FLUSHED_LEFT, 0, 3);
      sprintf (points, "%u", highs[i][j].points);
      points_txt[i][j] = compile_menu_text (points, T_FLUSHED_RIGHT,
					    0, 316);
    }

  i = 0;
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
      DRAW_SPRITE (higher_scores_txt, corner[0]);

      if (!rollflag) {
	DRAW_SPRITE (gamemode_txt[i], corner[0] + 40 * xbuf);
	for (j = 0; j < 10; j++) {
	  DRAW_SPRITE (highs_txt[i][j], corner[0] + (68 + 13 * j) * xbuf);
	  DRAW_SPRITE (points_txt[i][j], corner[0] + (68 + 13 * j) * xbuf);
	}
      } else {
	draw_sprprog_clipped_left (gamemode_txt[i - 1],
				   corner[0] + 40 * xbuf - rolldec,
				   - rolldec, -30);
	for (j = 0; j < 10; j++) {
	  draw_sprprog_clipped_left (highs_txt[i - 1][j],
				     corner[0] + (68 + 13 * j) * xbuf
				     - rolldec, - rolldec, -30);
	  draw_sprprog_clipped_left (points_txt[i - 1][j],
				     corner[0] + (68 + 13 * j) * xbuf
				     - rolldec, - rolldec, -30);
	}
	draw_sprprog_clipped_right (gamemode_txt[i],
				    corner[0] + 40 * xbuf + 320 - rolldec,
				    320 - rolldec, 320);
	for (j = 0; j < 10; j++) {
	  draw_sprprog_clipped_right (highs_txt[i][j],
				      corner[0] + (68 + 13 * j) * xbuf
				      + 320 - rolldec, 320 - rolldec, 320);
	  draw_sprprog_clipped_right (points_txt[i][j],
				      corner[0] + (68 + 13 * j) * xbuf
				      + 320 - rolldec, 320 - rolldec, 320);
	}
      }
      hrule (28);
      hrule (59);
      flush_display (corner[0]);
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

  for (i = 0; i < 5; ++i)
    for (j = 0; j < 10; ++j) {
      FREE_SPRITE0 (points_txt[i][j]);
      FREE_SPRITE0 (highs_txt[i][j]);
    }
}

void
help_menu (void)
{
  keycode_t t;
  int top = 0;
#define MULT_SHIFT 10
  int curmult = 0;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();

    curmult += ((top << MULT_SHIFT) - curmult) / 8;

    draw_reader_data (help_text, corner[0] - 10 * xbuf,
		      curmult >> MULT_SHIFT, (curmult >> MULT_SHIFT) + 220);
    flush_display (corner[0]);

    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      switch (t) {
      case HK_Down:
	top += 10;
	break;
      case HK_Up:
	top -= 10;
	break;
      case HK_PageDown:
	top += 200;
	break;
      case HK_PageUp:
	top -= 200;
	break;
      case HK_End:
	top = help_text->max;
	break;
      case HK_Home:
	top = 0;
	break;
      default:
	/* NOP */
	break;
      }
      if (top + 200 > help_text->max / xbuf)
	top = help_text->max / xbuf - 200;
      if (top < 0)
	top = 0;
     } else
       t = 0;
  } while (t != HK_Escape);
  event_sfx (8);
}
