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
#include "const.h"
#include "sfx.h"
#include "options.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "keysdef.h"
#include "draw.h"
#include "heroes.h"
#include "structs.h"
#include "extras.h"
#include "display.h"
#include "savegame.h"
#include "hedlite.h"
#include "render.h"
#include "sound.h"
#include "txts.h"
#include "menus.h"
#include "keyb.h"
#include "misc.h"
#include "rsc_files.h"
#include "endian.h"
#include "fader.h"
#include "scrtools.h"
#include "sprrle.h"
#include "sprtext.h"
#include "sprprogwav.h"
#include "sound.h"

static sprite_t* left_arrow = 0;
static sprite_t* right_arrow = 0;
static sprite_t* checked_box[2] = {0, 0};
static sprite_t* cursor_bg = 0;
static sprite_t* cursor_fg = 0;
static sprite_t* horizontal_rule = 0;

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

static sprite_t* jukebox_frame = 0;
static sprite_t* jukebox_back = 0;
static sprite_t* jukebox_forw = 0;
static sprite_t* jukebox_quit = 0;


void
init_menus_sprites (void)
{
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

  /* control menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[90], T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (txti[91], T_FLUSHED_LEFT, 39, 56));
  add_sprprog0 (compile_menu_text (txti[92], T_FLUSHED_LEFT, 72, 56));
  add_sprprog0 (compile_menu_text (txti[93], T_FLUSHED_LEFT, 111, 56));
  add_sprprog0 (compile_menu_text (txti[92], T_FLUSHED_LEFT, 144, 56));
  add_sprprog0 (compile_menu_text (txti[94], T_FLUSHED_LEFT, 182, 56));
  control_menu_txt = end_sprprog ();

  /* sound menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[104], T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (txti[107], T_FLUSHED_LEFT, 39, 56));
  add_sprprog0 (compile_menu_text (txti[108], T_FLUSHED_LEFT, 109, 56));
  add_sprprog0 (compile_menu_text (txti[94], T_FLUSHED_LEFT, 179, 56));
  sound_menu_txt = end_sprprog ();

  music_vol_txt = compile_menu_text (txti[105], T_FLUSHED_LEFT, 74, 56);
  sfx_vol_txt = compile_menu_text (txti[106], T_FLUSHED_LEFT, 144, 56);

  /* screen menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[109], T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (txti[110], T_FLUSHED_LEFT, 34, 56));
  add_sprprog0 (compile_menu_text (txti[111], T_FLUSHED_LEFT, 63, 56));
  add_sprprog0 (compile_menu_text (txti[112], T_FLUSHED_LEFT, 92, 56));
  add_sprprog0 (compile_menu_text (txti[113], T_FLUSHED_LEFT, 121, 56));
  add_sprprog0 (compile_menu_text (txti[114], T_FLUSHED_LEFT, 150, 56));
  add_sprprog0 (compile_menu_text (txti[94], T_FLUSHED_LEFT, 179, 56));
  screen_menu_txt = end_sprprog ();

  /* game menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[115], T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (txti[116], T_FLUSHED_LEFT, 33, 56));
  add_sprprog0 (compile_menu_text (txti[117], T_FLUSHED_LEFT, 57, 56));
  add_sprprog0 (compile_menu_text (txti[118], T_FLUSHED_LEFT, 81, 56));
  add_sprprog0 (compile_menu_text (txti[119], T_FLUSHED_LEFT, 105, 56));
  add_sprprog0 (compile_menu_text (txti[120], T_FLUSHED_LEFT, 129, 56));
  add_sprprog0 (compile_menu_text (txti[94], T_FLUSHED_LEFT, 177, 56));
  game_menu_txt = end_sprprog ();

  /* keyboard menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[95], T_CENTERED|T_WAVING, 5, 159));
  add_sprprog0 (compile_menu_text (txti[94], T_CENTERED, 188, 159));
  /* 1st player */
  add_sprprog0 (compile_menu_text (txti[96], T_CENTERED, 25, 159));
  add_sprprog0 (compile_menu_text (txti[97], T_FLUSHED_LEFT, 38, 25));
  add_sprprog0 (compile_menu_text (txti[98], T_FLUSHED_LEFT, 60, 25));
  add_sprprog0 (compile_menu_text (txti[99], T_FLUSHED_LEFT, 49, 25));
  add_sprprog0 (compile_menu_text (txti[100], T_FLUSHED_LEFT, 71, 25));
  add_sprprog0 (compile_menu_text (txti[101], T_FLUSHED_LEFT, 82, 25));
  add_sprprog0 (compile_menu_text (txti[102], T_FLUSHED_LEFT, 93, 25));
  /* 2nd player */
  add_sprprog0 (compile_menu_text (txti[103], T_CENTERED, 108, 159));
  add_sprprog0 (compile_menu_text (txti[97], T_FLUSHED_LEFT, 121, 25));
  add_sprprog0 (compile_menu_text (txti[98], T_FLUSHED_LEFT, 143, 25));
  add_sprprog0 (compile_menu_text (txti[99], T_FLUSHED_LEFT, 132, 25));
  add_sprprog0 (compile_menu_text (txti[100], T_FLUSHED_LEFT, 154, 25));
  add_sprprog0 (compile_menu_text (txti[101], T_FLUSHED_LEFT, 165, 25));
  add_sprprog0 (compile_menu_text (txti[102], T_FLUSHED_LEFT, 176, 25));
  keyboard_menu_txt = end_sprprog ();

  /* extra menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text (txti[125], T_CENTERED|T_WAVING, 5, 169));
  add_sprprog0 (compile_menu_text (txti[94], T_FLUSHED_LEFT, 180, 20));
  extra_menu_txt = end_sprprog ();
  extra_modes_txt[0] = compile_menu_text (txti[122], T_FLUSHED_LEFT, 35, 20);
  extra_modes_txt[1] = compile_menu_text (txti[123], T_FLUSHED_LEFT, 35, 20);
  extra_modes_txt[2] = compile_menu_text (txti[124], T_FLUSHED_LEFT, 35, 20);
  extra_combine_txt[0] = compile_menu_text (txti[126], T_FLUSHED_LEFT, 57, 20);
  extra_combine_txt[1] = compile_menu_text (txti[127], T_FLUSHED_LEFT, 57, 20);
  extra_combine_txt[2] = compile_menu_text (txti[128], T_FLUSHED_LEFT, 57, 20);

  /* credit menu */
  new_sprprog ();
  add_sprprog0 (compile_menu_text ("CREDITS", T_CENTERED|T_WAVING, 10, 159));
  add_sprprog0 (compile_menu_text ("GFX AND IDEA:", T_FLUSHED_LEFT, 40, 1));
  add_sprprog0 (compile_menu_text ("a GUEN",
				   T_FLUSHED_RIGHT|T_WAVING, 40, 318));
  add_sprprog0 (compile_menu_text ("MUSIC:", T_FLUSHED_LEFT, 60, 1));
  add_sprprog0 (compile_menu_text ("b TNK",
				   T_FLUSHED_RIGHT|T_WAVING, 60, 318));
  add_sprprog0 (compile_menu_text ("c ALEXEL",
				   T_FLUSHED_RIGHT|T_WAVING, 72, 318));
  add_sprprog0 (compile_menu_text ("CODE:", T_FLUSHED_LEFT, 93, 1));
  add_sprprog0 (compile_menu_text ("b POLLUX",
				   T_FLUSHED_RIGHT|T_WAVING, 93, 318));
  /* FIXME: rewrite when a paragraph formating function exists */
  add_sprprog0 (compile_menu_text ("SEE THE FILE", T_CENTERED, 118, 159));
  add_sprprog0 (compile_menu_text ("THANKS", T_CENTERED, 130, 159));
  add_sprprog0 (compile_menu_text ("FOR OTHER", T_CENTERED, 142, 159));
  add_sprprog0 (compile_menu_text ("CONTRIBUTORS", T_CENTERED, 154, 159));
  credit_menu_txt = end_sprprog ();

  jukebox_frame = compile_sprrle (IMGPOS (jukebox_img, 0, 0), 0,
				  19, 306, jukebox_img.width, xbuf);
  jukebox_forw = compile_sprrle (IMGPOS (jukebox_img, 19, 0), 0,
				 9, 12, jukebox_img.width, xbuf);
  jukebox_back = compile_sprrle (IMGPOS (jukebox_img, 19, 12), 0,
				 9, 12, jukebox_img.width, xbuf);
  jukebox_quit = compile_sprrle (IMGPOS (jukebox_img, 19, 24), 0,
				 9, 16, jukebox_img.width, xbuf);
}

void
uninit_menus_sprites (void)
{
  FREE_SPRITE0 (left_arrow);
  FREE_SPRITE0 (right_arrow);
  FREE_SPRITE0 (checked_box[0]);
  FREE_SPRITE0 (checked_box[1]);
  FREE_SPRITE0 (cursor_bg);
  FREE_SPRITE0 (cursor_fg);
  FREE_SPRITE0 (horizontal_rule);

  FREE_SPRITE0 (control_menu_txt);
  FREE_SPRITE0 (sound_menu_txt);
  FREE_SPRITE0 (music_vol_txt);
  FREE_SPRITE0 (sfx_vol_txt);
  FREE_SPRITE0 (game_menu_txt);
  FREE_SPRITE0 (game_rounds_txt);
  FREE_SPRITE0 (screen_menu_txt);
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
control_menu (void)
{
  int l = 0;
  keycode_t t;

  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    copy_rect_4 (icons_img.buffer + (106 + 19 * opt.ctrl_one) * 320,
		 corner[0] + 35 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.autopilot_one) * 320 + 144,
		 corner[0] + 68 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (106 + 19 * opt.ctrl_two) * 320,
		 corner[0] + 107 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.autopilot_two) * 320 + 144,
		 corner[0] + 140 * xbuf + 20, 32, 18);
    arrows (35 + l * 35 - 2 * (l == 1) + 2 * (l == 2 || l == 4), 1);
    chkbox (69, 260, opt.autopilot_one);
    chkbox (141, 260, opt.autopilot_two);
    hrule (95);
    DRAW_SPRITE (control_menu_txt, corner[0]);
    vsynch ();
    aff_buffer ();
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
  keycode_t t;
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
      if (l != reorder[i] || testing == 0) {
	if (!keyboard_keys_txt[i]) { /* need to generate a sprite ? */
	  sprite_t *res;
	  int key = opt.player_keys[i > 5][i > 5 ? i - 6 : i];
	  const char *keyname = search_keyname (key);

	  if (!keyname) {	/* unknown key? print its code number */
	    char name[64];
	    sprintf (name, "(%d)", key);
	    res = compile_menu_text (name, T_FLUSHED_RIGHT,
				     keyline[reorder[i]], 295);
	  } else
	    res = compile_menu_text (keyname, T_FLUSHED_RIGHT,
				     keyline[reorder[i]], 295);
	  keyboard_keys_txt[i] = res;
	}
	DRAW_SPRITE (keyboard_keys_txt[i], corner[0]);
      }
    }
    vsynch ();
    aff_buffer ();

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
	l++;

	/* force the regeneration of key name on next display */
	FREE_SPRITE0 (keyboard_keys_txt[l]);
	keyboard_keys_txt[l] = 0;

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
    copy_rect_4 (icons_img.buffer + 163 * 320 + 252,
		 corner[0] + 35 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + 182 * 320 + 252,
		 corner[0] + 105 * xbuf + 20, 32, 18);
    arrows (35 + l * 35, 1);
    chkbox (36, 260, opt.music);
    chkbox (106, 260, opt.sfx);
    if (opt.music) {
      copy_rect_4 (icons_img.buffer +
		   (68 + 19 * (opt.music_volume / 2)) * 320 + 36,
		   corner[0] + 70 * xbuf + 20, 32, 18);
      cursor (72, 251, 13 - opt.music_volume, 13);
      DRAW_SPRITE (music_vol_txt, corner[0]);
    }
    if (opt.sfx) {
      copy_rect_4 (icons_img.buffer + (68 + 19 * (opt.sfx_volume / 2)) * 320 +
		   72, corner[0] + 140 * xbuf + 20, 32, 18);
      cursor (142, 251, 13 - opt.sfx_volume, 13);
      DRAW_SPRITE (sfx_vol_txt, corner[0]);
    }
    DRAW_SPRITE (sound_menu_txt, corner[0]);
    vsynch ();
    aff_buffer ();
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

    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.radar_map) * 320 + 36,
		 corner[0] + 30 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.use_glenz) * 320 + 108,
		 corner[0] + 59 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.display_infos) * 320 + 180,
		 corner[0] + 88 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * opt.luminance) * 320 + 252,
		 corner[0] + 117 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (68 + 19 * opt.inertia) * 320 + 108,
		 corner[0] + 146 * xbuf + 20, 32, 18);

    arrows (30 + l * 29, 1);
    chkbox (31, 260, opt.radar_map);
    chkbox (60, 260, opt.use_glenz);
    chkbox (89, 260, opt.display_infos);

    cursor (120, 251, 6 - opt.luminance, 6);
    chkbox (147, 260, opt.inertia);
    DRAW_SPRITE (screen_menu_txt, corner[0]);

    vsynch ();
    aff_buffer ();
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      t = move_updown (t, &l, 5);
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l < 5) {
	  if (l == 0)
	    opt.radar_map ^= 1;
	  else if (l == 1)
	    opt.use_glenz ^= 1;
	  else if (l == 2)
	    opt.display_infos ^= 1;
	  else if (l == 3) {
	    if (t == HK_Right && opt.luminance > 0)
	      --opt.luminance;
	    else if (t == HK_Left && opt.luminance < 6)
	      ++opt.luminance;
	    set_pal_with_luminance (&tile_set_img.palette);
	  } else if (l == 4)
	    opt.inertia ^= 1;

	  if (l == 3)
	    event_sfx (4);
	  else
	    event_sfx (3);
	}
    } else
      t = 0;
  } while (t != HK_Enter || l != 5);
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
    copy_rect_4 (icons_img.buffer + (68 + 19 * opt.player_color[0]) * 320 +
		 144, corner[0] + 29 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (68 + 19 * opt.player_color[1]) * 320 +
		 144, corner[0] + 53 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (68 + 19 * opt.player_color[2]) * 320 +
		 144, corner[0] + 77 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (68 + 19 * opt.player_color[3]) * 320 +
		 144, corner[0] + 101 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + (11 + 19 * (4 - opt.speed * 2)) * 320 +
		 216, corner[0] + 125 * xbuf + 20, 32, 18);

    copy_rect_4 (icons_img.buffer + (68 + 19 * 4) * 320 + 144,
		 corner[0] + 149 * xbuf + 20, 32, 18);
    cursor (128, 251, opt.speed, 2);
    cursor (154, 251, opt.gamerounds, 15);
    arrows (29 + l * 24 + 24 * (l > 2), 1);
    DRAW_SPRITE (game_menu_txt, corner[0]);

    if (!game_rounds_txt) {
      char rounds[32];
      sprintf (rounds, txti[121], rounds_nbr_values[opt.gamerounds],
	       (opt.gamerounds == 0) ? '\0' : 'S');
      game_rounds_txt = compile_menu_text (rounds, T_FLUSHED_LEFT, 153, 56);
    }
    DRAW_SPRITE (game_rounds_txt, corner[0]);

    vsynch ();
    aff_buffer ();
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
    vsynch ();
    aff_buffer ();
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
  int l = 0;
  int t;

  do {
    std_white_fadein (&tile_set_img.palette);
    do {
      background_menu ();
      draw_text_waving (txti[129], 159, 12, 1);
      draw_text_array[l == 0] (txti[130], 159, 55, 1);
      draw_text_array[l == 1] (txti[131], 159, 75, 1);
      draw_text_array[l == 2] (txti[132], 159, 95, 1);
      draw_text_array[l == 3] (txti[133], 159, 115, 1);
      draw_text_array[l == 4] (txti[134], 159, 135, 1);
      draw_text_array[l == 5] (txti[136], 159, 155, 1);
      draw_text_array[l == 6] (txti[94], 159, 175, 1);
      hrule (68);
      hrule (88);
      hrule (108);
      hrule (128);
      hrule (148);
      hrule (168);
      waving_arrows (50 + l * 20, 70);
      vsynch ();
      aff_buffer ();
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
	t = move_updown (t, &l, 6);
      } else
	t = 0;
    } while (t != HK_Enter);
    if (l != 6) {
      event_sfx (2);
      if (l == 0)
	game_menu ();
      else if (l == 1)
	screen_menu ();
      else if (l == 2)
	sound_menu ();
      else if (l == 3)
	control_menu ();
      else if (l == 4)
	keyboard_menu ();
      else /* l == 5 */ {
	if (extra_nbr > 0)
	  extra_menu ();
      }
    }
  } while (l != 6);
  event_sfx (8);
}

void
draw_quit_menu (int l)
{
  draw_text_waving (txti[140], 159, 75, 1);
  draw_text_array[l == 0] (txti[142], 159, 95, 1);
  draw_text_array[l == 1] (txti[141], 159, 110, 1);
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
    vsynch ();
    aff_buffer ();
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
  draw_text_waving (txti[145], 159, 4, 1);
  hrule (21);
  if (two_players)
    draw_text_array[l == 0] (txti[146], 159, 31, 1);

  else
    draw_text_array[l == 0] (txti[147], 159, 31, 1);
  hrule (48);
  draw_text_array[l == 1] (mode_name[0], 159, 60, 1);
  draw_text_array[l == 2] (mode_name[2], 159, 83, 1);
  draw_text_array[l == 3] (mode_name[1], 159, 99, 1);
  draw_text_array[l == 4] (mode_name[3], 159, 115, 1);
  draw_text_array[l == 5] (mode_name[4], 159, 131, 1);
  hrule (150);
  draw_text_array[l == 6] (txti[148], 159, 160, 1);
  hrule (177);
  draw_text_array[l == 7] (txti[94], 159, 187, 1);
  waving_arrows (41 + l * 16 + 5 * (l > 1) - 15 * (l == 0) +
		 13 * (l == 6) + 23 * (l == 7), 30);
}

void
draw_main_menu (int l)
{
  draw_text_waving (txti[150], 159, 12, 1);
  draw_text_array[l == 0] (txti[151], 159, 55, 1);
  draw_text_array[l == 1] (txti[152], 159, 75, 1);
  draw_text_array[l == 2] (txti[153], 159, 95, 1);
  draw_text_array[l == 3] (txti[154], 159, 115, 1);
  draw_text_array[l == 4] (txti[155], 159, 135, 1);
  draw_text_array[l == 5] (txti[157], 159, 155, 1);
  draw_text_array[l == 6] (txti[158], 159, 175, 1);
  hrule (68);
  hrule (88);
  hrule (108);
  hrule (128);
  hrule (148);
  hrule (168);
  waving_arrows (50 + l * 20, 75);
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
  char lname[FILENAME_SIZE + 1];

  if (extra_user_nbr == 1) {
    event_sfx (116);
    strcpy (lname, extra_list[0].level_name);

    hmain (lname, 0, 0, 0, 0, 0);
    return;
  }
  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();
    draw_text_waving (txti[170], 159, 10, 1);
    hrule (30);
    hrule (187);
    for (i = -5; i <= 5; i++)
      if ((i + l) >= 0 && (unsigned int) (i + l) < extra_user_nbr) {
	strcpy (lname, extra_list[i + l].level_name);
	draw_text_array[i == 0] (lname, 159, 105 + i * 13, 1);
      }
    waving_arrows (101, 60);
    vsynch ();
    aff_buffer ();
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape || t == HK_Home
	  || t == HK_End || t == HK_PageUp || t == HK_PageDown)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > 0)
	  l--;
	else
	  l = extra_user_nbr - 1;
      }
      if (t == HK_Down) {
	if ((unsigned int) (l + 1) < extra_user_nbr)
	  l++;
	else
	  l = 0;
      }
      if (t == HK_Home)
	l = 0;
      if (t == HK_End)
	l = extra_user_nbr - 1;
      if (t == HK_PageUp)
	l = (l > 10) ? (l - 10) : 0;
      if (t == HK_PageDown)
	l = (((unsigned int) (l + 11) < extra_user_nbr)
	     ? (l + 10) : (extra_user_nbr - 1));
    } else
      t = 0;
  } while (t != HK_Enter && t != HK_Escape);
  if (t == HK_Enter) {
    event_sfx (116);
    strcpy (lname, extra_list[l].level_name);

    hmain (lname, 0, 0, 0, 0, 0);
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

  memset (lname,0,FILENAME_SIZE + 1);
  pcx_load_from_rsc ("new-level-menu-img", (pcx_image_t *) & frmenu);
  load_tile_set_preview (0, (pcx_image_t *) & tilesprev);
  for (i = 0; i < 52; i++)
    memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
	    tilesprev.buffer + i * 62, 62);
  memcpy (corner[0], frmenu.buffer, 64000);
  corner[0] = render_buffer[0];
  std_white_fadein (&tilesprev.palette);
  do {
    update_text_waving_step ();
    j = minisinus[read_htimer (waving_htimer) & 31];
    memcpy (corner[0], frmenu.buffer, 64000);
    copy_rect_transp_320 (main_font_img.buffer + 218 + 50 * 320 +
			  (xwrap != -1) * 21, corner[0] + 82 * 320 + 170, 21,
			  14);
    copy_rect_transp_320 (main_font_img.buffer + 218 + 50 * 320 +
			  (ywrap != -1) * 21, corner[0] + 103 * 320 + 170, 21,
			  14);
    copy_rect_transp_320 (main_font_img.buffer + 173 + 51 * 320,
			  corner[0] + 135 * 320 + 153 + xsize / 2 - 5, 8, 14);
    copy_rect_transp_320 (main_font_img.buffer + 173 + 51 * 320,
			  corner[0] + 156 * 320 + 153 + ysize / 2 - 5, 8, 14);
    if (l == 0)
      copy_rect_4_320 (main_font_img.buffer + 50 * 320 + 260,
		       corner[0] + 271 + 32 * 320, 16, 11);
    if (l == 1)
      copy_rect_4_320 (main_font_img.buffer + 81 * 320 + 32,
		       corner[0] + 275 + 56 * 320, 8, 5);
    if (l == 2)
      copy_rect_4_320 (main_font_img.buffer + 81 * 320 + 32,
		       corner[0] + 194 + 87 * 320, 8, 5);
    if (l == 3)
      copy_rect_4_320 (main_font_img.buffer + 81 * 320 + 32,
		       corner[0] + 194 + 108 * 320, 8, 5);
    if (l == 4)
      copy_rect_4_320 (main_font_img.buffer + 81 * 320 + 32,
		       corner[0] + 194 + 139 * 320, 8, 5);
    if (l == 5)
      copy_rect_4_320 (main_font_img.buffer + 81 * 320 + 32,
		       corner[0] + 194 + 160 * 320, 8, 5);
    if (l == 6) {
      copy_rect_transp_320 (main_font_img.buffer + 60 * 320 + 147,
			    corner[0] + 214 + 186 * 320 + j, 10, 10);
      copy_rect_transp_320 (main_font_img.buffer + 60 * 320 + 157,
			    corner[0] + 299 + 186 * 320 - j, 10, 10);
    } else {
      copy_rect_transp_320 (main_font_img.buffer + 50 * 320 + 147,
			    corner[0] + 214 + 186 * 320, 10, 10);
      copy_rect_transp_320 (main_font_img.buffer + 50 * 320 + 157,
			    corner[0] + 299 + 186 * 320, 10, 10);
    }
    if (flaglock)
      draw_text_waving_320 (txti[174], 159, 7, 1);

    else
      draw_text_waving_320 (txti[175], 159, 7, 1);

    /* draw_text_array_320[l==0](txti[176],8,33,0); */
    draw_text_320 ((char *) titres[tiles], 138, 33, 1);
    draw_text_array_320[l == 1] (txti[177], 8, 54, 0);
    draw_text_320 ((char *) lname, 185, 54, 1);
    draw_text_array_320[l == 2] (txti[178], 8, 85, 0);
    draw_text_array_320[l == 3] (txti[179], 8, 106, 0);
    draw_text_array_320[l == 4] (txti[180], 8, 137, 0);
    draw_text_array_320[l == 5] (txti[181], 8, 158, 0);
    draw_text_array_320[l == 6] (txti[182], 227, 186, 0);
    sprintf (ssize, "%d", xsize);
    draw_text_320 (ssize, 248, 137, 1);
    sprintf (ssize, "%d", ysize);
    draw_text_320 (ssize, 248, 158, 1);
    vsynch ();
    memcpy (screen, corner[0], 64000); /* FIXME: what is this? */
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
	  }
	}
	if ((t == HK_BackSpace || t == HK_Delete) && (pos > 0)) {
	  pos--;
	  lname[pos] = 0;
	  event_sfx (113);
	  flag = 1;
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
      }
    } else
      t = 0;
    if (l == 6 && t == HK_Enter && lname[0] == 0)
      l = 1;
  } while (t != HK_Escape && !(l == 6 && t == HK_Enter));
  img_free (&frmenu);
  img_free (&tilesprev);
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
    draw_text_waving (txti[171], 159, 10, 1);
    draw_text_array[l == 0] (txti[172], 159, 85, 1);
    draw_text_array[l == 1] (txti[173], 159, 105, 1);
    waving_arrows (81 + l * 20, 50);
    vsynch ();
    aff_buffer ();
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > 0)
	  l = 0;
	else
	  l = 1;
      }
      if (t == HK_Down) {
	if (l < 5)
	  l = 1;
	else
	  l = 0;
      }
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
    } else
      event_sfx (8);
  }
}

void
draw_saved_games_info (int decal, int l, char h)
{
  int i;
  char c;
  if (h)
    draw_text_waving (txti[185], 159 + decal, 15, 1);

  else
    draw_text_waving (txti[186], 159 + decal, 15, 1);
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
  for (i = 0; i < 10; i++)
    draw_text (saverec[i].name, 159 + decal, 40 + i * 14, 1);

  DRAW_SPRITE (left_arrow, corner[0] + decal + (35 + l * 14) * xbuf + 1);
  DRAW_SPRITE (right_arrow,
	       corner[0] + decal + (35 + l * 14) * xbuf + 320 - 1 - 13);
}

void
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

  vsynch ();
  aff_buffer ();
}

int
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
  signed char sinl;
  int l = 0;
  htimer_t lemming_htimer = new_htimer (T_GLOBAL, HZ (18));

  in_jokebox = 1;
  std_white_fadein (&tile_set_img.palette);
  do {
    background_menu ();

    sinl = minisinus[read_htimer (waving_htimer) & 31];
    draw_glenz_box (corner[0] + (42 + sinl) * xbuf + 234, 2, 86, 6);
    draw_glenz_box (corner[0] + (62 + sinl) * xbuf + 244, 3, 76, 6);
    draw_glenz_box (corner[0] + (74 + sinl) * xbuf + 194, 4, 126, 6);
    draw_glenz_box (corner[0] + (95 + sinl) * xbuf + 194, 5, 126, 6);
    DRAW_SPRITE (credit_menu_txt, corner[0]);
    hrule (28);
    hrule (109);
    hrule (171);
    {
      int lempos = read_htimer (lemming_htimer);
      copy_rect_transp (main_font_img.buffer +
			81 * 320 + 132 + 6 * (lempos & 7),
			corner[0] + (190) * xbuf + (lempos / 2) - 6, 6, 10);
      if ((lempos / 2) >= 332)
	reset_htimer (lemming_htimer);
    }
    jukebox_draw (l);
  } while (jukebox_keys (&l));

  event_sfx (76);
  in_jokebox = 0;
  free_htimer (lemming_htimer);
}
