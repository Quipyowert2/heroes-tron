/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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


#include <stdlib.h>
#include <stdio.h>
#include "const.h"
#include "sfx.h"
#include "options.h"
//#include "font_menu.h"
#include "keyb.h"
#include "keys_heroes.h"
#include "keysdef.h"
#include "draw.h"
#include "heroes.h"
#include <string.h>
#include <math.h>
#include "structs.h"
#include "extras.h"
#include "display.h"
#include "savegame.h"
#include "hedlite.h"
#include "render.h"
#include "sound.h"
#include "txts.h"

#include "menus.h"
#include "config.h"
#include "keyb.h"
#include "misc.h"
#include "rsc_files.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif
#include "endian.h"

void
background_menu (void)
{
  static long int TTT = 0;
  for (; frame_old < frame_cur; frame_old++) {
    TTT++;
    if (p > 0)
      p--;
    if (p2 < 0)
      p2++;

    else if (p2 > 0)
      p2--;
  }
  camera_x[0] = 65536 * 24 * cos (TTT / (111.0 /**1.2*/ ));
  camera_y[0] = 65536 * 24 * sin (TTT / (175.0 /**1.2*/ ));
  *(((short int *) &camera_x[0]) + 1) &= (short int) map_info.xwrap;
  *(((short int *) &camera_y[0]) + 1) &= (short int) map_info.ywrap;
  inert_x[0] = camera_x[0];
  inert_y[0] = camera_y[0];
  compute_corner (0, 1);
  draw_level (0);
};

static void
control_menu (void)
{
  char l = 0;
  int t;
  p = 64;
  memset (pal.global, 63, 768);

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
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (35 + l * 35 - 2 * (l == 1) +
				   2 * (l == 2
					|| l == 4)) * xbuf + 1, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (35 + l * 35 - 2 * (l == 1) +
				   2 * (l == 2
					|| l == 4)) * xbuf + 320 - 1 - 13, 13,
		      20);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.autopilot_one * 21, corner[0] + 69 * xbuf + 260, 21,
		      14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.autopilot_two * 21, corner[0] + 141 * xbuf + 260,
		      21, 14);
    copy_rect_transp (main_font_img.buffer + 61 * 320,
		      corner[0] + 95 * xbuf + 100, 120, 3);
    draw_text_waving (txti[90], 159, 5, 1);
    draw_text (txti[91], 56, 39, 0);
    draw_text (txti[92], 56, 72, 0);
    draw_text (txti[93], 56, 111, 0);
    draw_text (txti[92], 56, 144, 0);
    draw_text (txti[94], 56, 182, 0);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();

//   if (p>=0) set_pal((char *)&temppal.global,0,768);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > 0)
	  l--;
	else
	  l = 4;
      }
      if (t == HK_Down) {
	if (l < 4)
	  l++;
	else
	  l = 0;
      }
      if (t == HK_Escape) {
	if (l != 4)
	  l = 4;
	else
	  t = HK_Enter;
      }
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
search_keyname (int key)
{
  const struct keynames_s* k = keynames;

  while (k->name && k->code != key)
    ++k;
  return k->name;
}

static void
draw_key (int key, int line)
{
  const char* keyname = search_keyname (key);
  
  if (!keyname) {
    sprintf (tmp1, "(%d)", key);
    draw_text (tmp1, 295, line, 2);
  } else 
    draw_text (keyname, 295, line, 2);
}

static void
keyboard_menu (void)
{
  char l = 0, testing = 0;
  int ll, i, t;
  int unconfigured_keys = -1;
  
  p = 64;
  memset (pal.global, 63, 768);

  do {
    background_menu ();

//   copy_rect_transp(main_font_img.buffer+61*320,corner[0]+99*xbuf+100,120,3);
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (11 * l + 33 + 15 * (l >= 6) +
				   4 * (l == 12)) * xbuf + 1, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (11 * l + 33 + 15 * (l >= 6) +
				   4 * (l == 12)) * xbuf + 320 - 1 - 13, 13,
		      20);
    draw_text_waving (txti[95], 159, 5, 1);
    draw_text (txti[96], 159, 25, 1);
    draw_text (txti[97], 25, 38, 0);
    if (testing == 0 || l != 0)
      draw_key (opt.player_keys[0][0], 38);
    draw_text (txti[98], 25, 60, 0);
    if (testing == 0 || l != 2)
      draw_key (opt.player_keys[0][1], 60);
    draw_text (txti[99], 25, 49, 0);
    if (testing == 0 || l != 1)
      draw_key (opt.player_keys[0][2], 49);
    draw_text (txti[100], 25, 71, 0);
    if (testing == 0 || l != 3)
      draw_key (opt.player_keys[0][3], 71);
    draw_text (txti[101], 25, 82, 0);
    if (testing == 0 || l != 4)
      draw_key (opt.player_keys[0][4], 82);
    draw_text (txti[102], 25, 93, 0);
    if (testing == 0 || l != 5)
      draw_key (opt.player_keys[0][5], 93);
    draw_text (txti[103], 159, 108, 1);
    draw_text (txti[97], 25, 121, 0);
    if (testing == 0 || l != 6)
      draw_key (opt.player_keys[1][0], 121);
    draw_text (txti[98], 25, 143, 0);
    if (testing == 0 || l != 8)
      draw_key (opt.player_keys[1][1], 143);
    draw_text (txti[99], 25, 132, 0);
    if (testing == 0 || l != 7)
      draw_key (opt.player_keys[1][2], 132);
    draw_text (txti[100], 25, 154, 0);
    if (testing == 0 || l != 9)
      draw_key (opt.player_keys[1][3], 154);
    draw_text (txti[101], 25, 165, 0);
    if (testing == 0 || l != 10)
      draw_key (opt.player_keys[1][4], 165);
    draw_text (txti[102], 25, 176, 0);
    if (testing == 0 || l != 11)
      draw_key (opt.player_keys[1][5], 176);
    draw_text (txti[94], 159, 188, 1);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    ll = l;
    if (l == 1)
      ll = 2;

    else if (l == 2)
      ll = 1;

    else if (l == 7)
      ll = 8;

    else if (l == 8)
      ll = 7;
    if (testing == 0) {
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
	if (t == HK_Up || t == HK_Down || t == HK_Escape)
	  event_sfx (1);
	if (t == HK_Up) {
	  if (l > 0)
	    l--;
	  else
	    l = 12;
	}
	if (t == HK_Down) {
	  if (l < 12)
	    l++;
	  else
	    l = 0;
	}
	if (t == HK_Escape) {
	  if (l != 12)
	    l = 12;
	  else
	    t = HK_Enter;
	}
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
  char l = 0;
  int t;
  p = 64;
  memset (pal.global, 63, 768);

  do {
    background_menu ();
    copy_rect_4 (icons_img.buffer + 163 * 320 + 252,
		 corner[0] + 35 * xbuf + 20, 32, 18);
    copy_rect_4 (icons_img.buffer + 182 * 320 + 252,
		 corner[0] + 105 * xbuf + 20, 32, 18);
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (35 + l * 35) * xbuf + 1, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (35 + l * 35) * xbuf + 320 - 1 - 13, 13,
		      20);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 + opt.music * 21,
		      corner[0] + 36 * xbuf + 260, 21, 14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 + opt.sfx * 21,
		      corner[0] + 106 * xbuf + 260, 21, 14);
    if (opt.music) {
      copy_rect_4 (icons_img.buffer +
		   (68 + 19 * (opt.music_volume / 2)) * 320 + 36,
		   corner[0] + 70 * xbuf + 20, 32, 18);
      copy_rect_transp (main_font_img.buffer + 181 + 53 * 320,
			corner[0] + 74 * xbuf + 251, 37, 10);
      copy_rect_transp (main_font_img.buffer + 173 + 51 * 320,
			corner[0] + 72 * xbuf + 253 + 2 + 2 * (11 -
							       opt.
							       music_volume),
			8, 14);
      draw_text (txti[105], 56, 74, 0);
    }
    if (opt.sfx) {
      copy_rect_4 (icons_img.buffer + (68 + 19 * (opt.sfx_volume / 2)) * 320 +
		   72, corner[0] + 140 * xbuf + 20, 32, 18);
      copy_rect_transp (main_font_img.buffer + 181 + 53 * 320,
			corner[0] + 144 * xbuf + 251, 37, 10);
      copy_rect_transp (main_font_img.buffer + 173 + 51 * 320,
			corner[0] + 142 * xbuf + 253 + 2 + 2 * (11 -
								opt.
								sfx_volume),
			8, 14); draw_text (txti[106], 56, 144, 0);
    }
    draw_text_waving (txti[104], 159, 5, 1);
    draw_text (txti[107], 56, 39, 0);
    draw_text (txti[108], 56, 109, 0);
    draw_text (txti[94], 56, 179, 0);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();

//   if (p>=0) set_pal((char *)&temppal.global,0,768);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > 0)
	  l--;
	else
	  l = 4;
	if (!opt.music && l == 1)
	  l = 0;
	if (!opt.sfx && l == 3)
	  l = 2;
      }
      if (t == HK_Down) {
	if (l < 4)
	  l++;
	else
	  l = 0;
	if (!opt.music && l == 1)
	  l = 2;
	if (!opt.sfx && l == 3)
	  l = 4;
      }
      if (t == HK_Escape) {
	if (l != 4)
	  l = 4;
	else
	  t = HK_Enter;
      }
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
  char l = /*0 */ 1;
  int t;
  p = 64;
  memset (pal.global, 63, 768);

  do {
    background_menu ();

//   copy_rect_4(icons_img.buffer+(11+19*opt.screen_size )*320    ,corner[0]+ 25*xbuf+20,32,18);
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
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (1 + l * 29) * xbuf + 1, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (1 + l * 29) * xbuf + 320 - 1 - 13, 13, 20);

//   copy_rect_transp(main_font_img.buffer+181+53*320,corner[0]+30*xbuf+251,37,10);
//   copy_rect_transp(main_font_img.buffer+173+51*320,corner[0]+28*xbuf+254+7*(3-opt.screen_size),8,14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.radar_map * 21, corner[0] + 31 * xbuf + 260, 21,
		      14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.use_glenz * 21, corner[0] + 60 * xbuf + 260, 21,
		      14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.display_infos * 21, corner[0] + 89 * xbuf + 260, 21,
		      14);
    copy_rect_transp (main_font_img.buffer + 181 + 53 * 320,
		      corner[0] + 122 * xbuf + 251, 37, 10);
    copy_rect_transp (main_font_img.buffer + 173 + 51 * 320,
		      corner[0] + 120 * xbuf + 253 + 4 * (6 - opt.luminance),
		      8, 14);
    copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
		      opt.inertia * 21, corner[0] + 147 * xbuf + 260, 21, 14);
    draw_text_waving (txti[109], 159, 5, 1);

//   draw_text("SIZE",56,29,0);
    draw_text (txti[110], 56, 34, 0);
    draw_text (txti[111], 56, 63, 0);
    draw_text (txti[112], 56, 92, 0);
    draw_text (txti[113], 56, 121, 0);
    draw_text (txti[114], 56, 150, 0);
    draw_text (txti[94], 56, 179, 0);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();

//   if (p>=0) set_pal((char *)&temppal.global,0,768);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > /*0 */ 1)
	  l--;
	else
	  l = 6;
      }
      if (t == HK_Down) {
	if (l < 6)
	  l++;
	else
	  l = /*0 */ 1;
      }
      if (t == HK_Escape) {
	if (l != 6)
	  l = 6;
	else
	  t = HK_Enter;
      }
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l < 6) {
	  if (l == 4)
	    event_sfx (4);
	  else
	    event_sfx (3);
	}
      if (t == HK_Right) {

//     if (l==0 && opt.screen_size>0) opt.screen_size--;
	if (l == 1)
	  opt.radar_map ^= 1;
	if (l == 2)
	  opt.use_glenz ^= 1;
	if (l == 3)
	  opt.display_infos ^= 1;
	if (l == 4 && opt.luminance > 0) {
	  opt.luminance--;
	  set_pal_with_luminance (&tile_set_img.palette);
	}
	if (l == 5)
	  opt.inertia ^= 1;
      }
      if (t == HK_Left) {

//     if (l==0 && opt.screen_size<3) opt.screen_size++;
	if (l == 1)
	  opt.radar_map ^= 1;
	if (l == 2)
	  opt.use_glenz ^= 1;
	if (l == 3)
	  opt.display_infos ^= 1;
	if (l == 4 && opt.luminance < 6) {
	  opt.luminance++;
	  set_pal_with_luminance (&tile_set_img.palette);
	}
	if (l == 5)
	  opt.inertia ^= 1;
      }
      if (t == HK_Enter) {

//     if (l==0) if (opt.screen_size<3) opt.screen_size++; else opt.screen_size=0;
	if (l == 1)
	  opt.radar_map ^= 1;
	if (l == 2)
	  opt.use_glenz ^= 1;
	if (l == 3)
	  opt.display_infos ^= 1;
	if (l == 4) {
	  if (opt.luminance > 0)
	    opt.luminance--;
	  else
	    opt.luminance = 6;
	}
	if (l == 4)
	  set_pal_with_luminance (&tile_set_img.palette);
	if (l == 5)
	  opt.inertia ^= 1;
      }
    } else
      t = 0;
  } while (t != HK_Enter || l != 6);
  event_sfx (8);
}

static void
game_menu (void)
{
  char l = 0, tmp;
  int t;
  p = 64;
  memset (pal.global, 63, 768);

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

//   copy_rect_4(icons_img.buffer+(163+19*opt.ghosts)*320+108,corner[0]+132*xbuf+20,32,18);
    copy_rect_4 (icons_img.buffer + (68 + 19 * 4) * 320 + 144,
		 corner[0] + 149 /*154 */  * xbuf + 20, 32, 18);
    copy_rect_transp (main_font_img.buffer + 181 + 53 * 320,
		      corner[0] + 130 * xbuf + 251, 37, 10);
    copy_rect_transp (main_font_img.buffer + 173 + 51 * 320,
		      corner[0] + 128 * xbuf + 255 + 5 * opt.speed * 2, 8,
		      14);

//   copy_rect_transp(main_font_img.buffer+218+50*320+opt.ghosts*21,corner[0]+133*xbuf+260,21,14);
    copy_rect_transp (main_font_img.buffer + 181 + 53 * 320,
		      corner[0] + 154 /*161 */  * xbuf + 251, 37, 10);
    copy_rect_transp (main_font_img.buffer + 173 + 51 * 320,
		      corner[0] + 152 /*159 */  * xbuf + 254 +
		      (3 * opt.gamerounds) / 2, 8, 14);
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (29 + l * 24 + 24 * (l > 2)) * xbuf + 1, 13,
		      20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (29 + l * 24 + 24 * (l > 2)) * xbuf + 320 -
		      1 - 13, 13, 20);
    draw_text_waving (txti[115], 159, 5, 1);
    draw_text (txti[116], 56, 33, 0);
    draw_text (txti[117], 56, 57, 0);
    draw_text (txti[118], 56, 81, 0);
    draw_text (txti[119], 56, 105, 0);
    draw_text (txti[120], 56, 129, 0);

//   draw_text("GHOSTS",56,149,0);
    sprintf (tmp1, txti[121], rounds_nbr_values[opt.gamerounds],
	     (opt.gamerounds == 0) ? '\0' : 'S');
    draw_text (tmp1, 56, 153 /*158 */ , 0);
    draw_text (txti[94], 56, 177 /*180 */ , 0);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (t == HK_Up) {
	if (l > 0)
	  l--;
	else
	  l = 5;
      }
      if (t == HK_Down) {
	if (l < 5)
	  l++;
	else
	  l = 0;
      }
      if (t == HK_Escape) {
	if (l != 5)
	  l = 5;
	else
	  t = HK_Enter;
      }
      if (t == HK_Right || t == HK_Left || t == HK_Enter)
	if (l != 6) {
	  if			/*(l==4) event_sfx(3);
				   else if */ ((l == 3) || (l == 4))
	    event_sfx (4);

	  else
	    event_sfx (5);
	}
      if (t == HK_Right || t == HK_Enter) {

//       if (l==4) opt.ghosts^=1;
	if (l == 3) {
	  if (opt.speed < 2)
	    opt.speed++;	/*else opt.speed=0; */
	}
	if (l == 4) {
	  if (opt.gamerounds < 15)
	    opt.gamerounds++;	/*else opt.gamerounds=0; */
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

//       if (l==4) opt.ghosts^=1;
	if (l == 3)
	  if (opt.speed > 0)
	    opt.speed--;
	if (l == 4)
	  if (opt.gamerounds > 0)
	    opt.gamerounds--;
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
  char l = 0;
  int t, i, ll = 0;

//  char *gtype[3]={txti[122],txti[123],txti[124]};
  p = 64;
  memset (pal.global, 63, 768);

  do {
    background_menu ();
    draw_text_waving (txti[125], 159, 5, 1);
    draw_text (txti[122 + opt.extras], 20, 35, 0);
    if (opt.extras == 0)
      draw_text (txti[126], 20, 57, 0);

    else {
      if (extrasel == 0)
	draw_text (txti[127], 20, 57, 0);

      else
	draw_text (txti[128], 20, 57, 0);
    }
    draw_text (txti[94], 20, 180, 0);
    if (extrasel && opt.extras != 0) {
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (72) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (169) * xbuf + 100, 120, 3);
      for (i = -3; i <= 3; i++)
	if ((i + ll) >= 0 && (i + ll) < extra_nbr) {
	  strcpy (tmp1, extra_list[i + ll].level_name);
	  draw_text_array[i == 0] (tmp1, 200, 118 + i * 13, 2);
	  copy_rect_transp (main_font_img.buffer + 218 + 50 * 320 +
			    extra_selected_list[i + ll] * 21,
			    corner[0] + 115 * xbuf + i * xbuf * 13 + 210, 21,
			    14);
	}
    }
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + (30 + l * 22 + 40 * (l == 2) +
				   78 * (l == 3)) * xbuf + 1, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] + (30 + l * 22 + 40 * (l == 2) +
				   78 * (l == 3)) * xbuf + 320 - 1 - 13, 13,
		      20);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();

//   if (p>=0) set_pal((char *)&temppal.global,0,768);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
    if (key_or_joy_ready ()) {
      t = get_key_or_joy ();
      if (t == HK_Up || t == HK_Down || t == HK_Escape)
	event_sfx (1);
      if (l == 2) {
	if (t == HK_Home || t == HK_End || t == HK_PageUp || t == HK_PageDown)
	  event_sfx (1);
	if (t == HK_Up) {
	  if (ll > 0)
	    ll--;
	  else
	    l = 1;
	}
	if (t == HK_Down) {
	  if (ll < (extra_nbr - 1))
	    ll++;
	  else
	    l = 3;
	}
	if (t == HK_Home)
	  ll = 0;
	if (t == HK_End)
	  ll = extra_nbr - 1;
	if (t == HK_PageUp)
	  ll = (ll > 10) ? (ll - 10) : 0;
	if (t == HK_PageDown)
	  ll = (ll < (extra_nbr - 11)) ? (ll + 10) : (extra_nbr - 1);
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
}

void
demo_info (void)
{
  int j, t;
  j = minisinus[frame_cur & 31];
  event_sfx (132);
  memset (pal.global, 63, 768);
  p = 64;

  do {
    background_menu ();
    draw_text_waving (txti[160], 159, 20, 1);
    draw_text (txti[161], 159, 60, 1);
    draw_text (txti[162], 159, 75, 1);
    draw_text (txti[163], 159, 90, 1);
    draw_text (txti[164], 159, 125, 1);
    draw_text (txti[165], 159, 140, 1);
    draw_text (txti[166], 159, 155, 1);
    vsynch ();
    pal2pal (&tile_set_img.palette, &pal, p);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (key_or_joy_ready ())
      t = get_key_or_joy ();

    else
      t = 0;
  } while (t == 0);
  event_sfx (8);
}

void
option_menu (void)
{
  char l = 0;
  int t, j;
  memset (pal.global, 63, 768);

  do {
    p = 64;

    do {
      background_menu ();
      j = minisinus[frame_cur & 31];
      draw_text_waving (txti[129], 159, 12, 1);
      draw_text_array[l == 0] (txti[130], 159, 55, 1);
      draw_text_array[l == 1] (txti[131], 159, 75, 1);
      draw_text_array[l == 2] (txti[132], 159, 95, 1);
      draw_text_array[l == 3] (txti[133], 159, 115, 1);
      draw_text_array[l == 4] (txti[134], 159, 135, 1);
      draw_text_array[l == 5] (txti[136], 159, 155, 1);
      draw_text_array[l == 6] (txti[94], 159, 175, 1);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (68 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (88 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (108 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (128 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (148 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 61 * 320,
			corner[0] + (168 /*+j */ ) * xbuf + 100, 120, 3);
      copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
			corner[0] + j + (50 + l * 20) * xbuf + 70, 13, 20);
      copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
			corner[0] - j + (50 + l * 20) * xbuf + 320 - 70 - 13,
			13, 20);
      pal2pal (&tile_set_img.palette, &pal, p);
      vsynch ();

//   if (p>=0) set_pal((char *)&temppal.global,0,768);
      if (p >= 0)
	set_pal_with_luminance ((palette_rvb *) temppal.global);
      aff_buffer ();
      if (p == 0)
	p--;
      if (key_or_joy_ready ()) {
	t = get_key_or_joy ();
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
      } else
	t = 0;
    } while (t != HK_Enter);
    if (l != 6)
      event_sfx (2);
    if (l == 0)
      game_menu ();
    if (l == 1)
      screen_menu ();
    if (l == 2)
      sound_menu ();
    if (l == 3)
      control_menu ();
    if (l == 4)
      keyboard_menu ();
    if (l == 5) {
      if (extra_nbr > 0)
	extra_menu ();
    }
  } while (l != 6);
  event_sfx (8);
}

void
draw_quit_menu (char l)
{
  int j;
  j = minisinus[frame_cur & 31];
  draw_text_waving (txti[140], 159, 75, 1);
  draw_text_array[l == 0] (txti[141], 159, 95, 1);
  draw_text_array[l == 1] (txti[142], 159, 110, 1);
  copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		    corner[0] + j + (91 + l * 15) * xbuf + 90, 13, 20);
  copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		    corner[0] - j + (91 + l * 15) * xbuf + 320 - 90 - 13, 13,
		    20);
}

char
quit_menu (void)
{
  char l = 0;
  int t;
  memset (pal.global, 63, 768);
  p = 64;

  do {
    background_menu ();
    draw_quit_menu (l);
    vsynch ();
    pal2pal (&tile_set_img.palette, &pal, p);
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
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
  if (t == HK_Escape || l == 0) {
    event_sfx (77);
    return (0);
  } else {
    event_sfx (78);
    return (1);
  }
}
void
draw_play_menu (char l)
{
  int j;
  background_menu ();
  j = minisinus[frame_cur & 31];
  draw_text_waving (txti[145], 159, 4, 1);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 21 * xbuf + 100, 120, 3);
  if (two_players)
    draw_text_array[l == 0] (txti[146], 159, 31, 1);

  else
    draw_text_array[l == 0] (txti[147], 159, 31, 1);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 48 * xbuf + 100, 120, 3);
  draw_text_array[l == 1] (mode_name[0], 159, 60, 1);

//   draw_text_array[l==2](mode_name[1],159,73,1);
  draw_text_array[l == 2] (mode_name[2], 159, 83, 1);
  draw_text_array[l == 3] (mode_name[1], 159, 99, 1);
  draw_text_array[l == 4] (mode_name[3], 159, 115, 1);
  draw_text_array[l == 5] (mode_name[4], 159, 131, 1);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 150 * xbuf + 100, 120, 3);
  draw_text_array[l == 6] (txti[148], 159, 160, 1);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 177 * xbuf + 100, 120, 3);
  draw_text_array[l == 7] (txti[94], 159, 187, 1);
  copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		    corner[0] + j + (41 + l * 16 + 5 * (l > 1) -
				     15 * (l == 0) + 13 * (l ==
							   6) + 23 * (l ==
								      7)) *
		    xbuf + 30, 13, 20);
  copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		    corner[0] - j + (41 + l * 16 + 5 * (l > 1) -
				     15 * (l == 0) + 13 * (l ==
							   6) + 23 * (l ==
								      7)) *
		    xbuf + 320 - 30 - 13, 13, 20);
  pal2pal (&tile_set_img.palette, &pal, p);
}

void
draw_main_menu (char l)
{
  int j;
  j = minisinus[frame_cur & 31];
  draw_text_waving (txti[150], 159, 12, 1);
  draw_text_array[l == 0] (txti[151], 159, 55, 1);
  draw_text_array[l == 1] (txti[152], 159, 75, 1);
  draw_text_array[l == 2] (txti[153], 159, 95, 1);
  draw_text_array[l == 3] (txti[154], 159, 115, 1);
  draw_text_array[l == 4] (txti[155], 159, 135, 1);
  draw_text_array[l == 5] (txti[157], 159, 155, 1);
  draw_text_array[l == 6] (txti[158], 159, 175, 1);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 68 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 88 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 108 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 128 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 148 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 61 * 320,
		    corner[0] + 168 * xbuf + 100, 120, 3);
  copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		    corner[0] + j + (50 + l * 20) * xbuf + 75, 13, 20);
  copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		    corner[0] - j + (50 + l * 20) * xbuf + 320 - 75 - 13, 13,
		    20);
  pal2pal (&tile_set_img.palette, &pal, p);
}

char tile_sets_names[10][3] =
  { "01", "02", "03", "04", "05", "06", "07", "08", "09", "10" };

static void
load_tile_set_preview (int num, image_ * ici)
{
  char *t = get_non_null_rsc_file ("editor-preview-prefix");
  strappend (t, tile_sets_names[num]);
  strappend (t, ".pcx");
  pcx_load (t, ici);
  free (t);
}

static void
editor_selector (void)
{
  int l = 0;
  int i = 0, t, j;
  if (extra_user_nbr == 1) {
    event_sfx (116);
    strcpy (tmp1, extra_list[0].level_name);

//      sprintf(tmp2,"%s A A A A A",tmp1);
//      spawnl(P_WAIT,"HEDLITE.EXE","HEDLITE.EXE",tmp2,NULL);

    hmain (7, tmp1, "A", "A", "A", "A", "A");
    return;
  }
  memset (pal.global, 63, 768);
  p = 64;

  do {
    background_menu ();
    j = minisinus[frame_cur & 31];
    draw_text_waving (txti[170], 159, 10, 1);
    copy_rect_transp (main_font_img.buffer + 61 * 320,
		      corner[0] + (30) * xbuf + 100, 120, 3);
    copy_rect_transp (main_font_img.buffer + 61 * 320,
		      corner[0] + (187) * xbuf + 100, 120, 3);
    for (i = -5; i <= 5; i++)
      if ((i + l) >= 0 && (i + l) < extra_user_nbr) {
	strcpy (tmp1, extra_list[i + l].level_name);
	draw_text_array[i == 0] (tmp1, 159, 105 + i * 13, 1);
      }
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + j + 101 * xbuf + 60, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] - j + 101 * xbuf + 320 - 60 - 13, 13, 20);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
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
	if (l < (extra_user_nbr - 1))
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
	l = (l < (extra_user_nbr - 11)) ? (l + 10) : (extra_user_nbr - 1);
    } else
      t = 0;
  } while (t != HK_Enter && t != HK_Escape);
  if (t == HK_Enter) {
    event_sfx (116);
    strcpy (tmp1, extra_list[l].level_name);

//    sprintf(tmp2,"%s A A A A A",tmp1);
//    spawnl(P_WAIT,"HEDLITE.EXE","HEDLITE.EXE",tmp2,NULL);

    hmain (7, tmp1, "A", "A", "A", "A", "A");
  } else
    event_sfx (8);
}

static void
editor_menu (void)
{
  image_ frmenu, tilesprev;
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
  char titres[10][16] =
    { "CORRIDOR 1", "CAEROS", "THE DARK AGES", "CORRIDOR 2", "VOLCANO",
    "ELECTRIC DREAM", "METAL MASTER", "MOON 51", "CORRIDOR 3",
    "SWEET DREAM"
  };
  tmp1[0] = 0;
  tmp1[1] = 0;
  tmp1[2] = 0;
  tmp1[3] = 0;
  tmp1[4] = 0;
  tmp1[5] = 0;
  tmp1[6] = 0;
  tmp1[7] = 0;
  tmp1[8] = 0;
  pcx_load_from_rsc ("new-level-menu-img", (image_ *) & frmenu);
  load_tile_set_preview (0, (image_ *) & tilesprev);
  for (i = 0; i < 52; i++)
    memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
	    tilesprev.buffer + i * 62, 62);
  memcpy (corner[0], frmenu.buffer, 64000);
  memset (pal.global, 63, 768);
  p = 64;
  corner[0] = render_buffer[0];

  do {
    j = minisinus[frame_cur & 31];
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
      affvga320sin (txti[174], 159, 7, 1);

    else
      affvga320sin (txti[175], 159, 7, 1);

//        draw_text_array_320[l==0](txti[176],8,33,0);
    draw_text_320 ((char *) titres[tiles], 138, 33, 1);
    draw_text_array_320[l == 1] (txti[177], 8, 54, 0);
    draw_text_320 ((char *) tmp1, 185, 54, 1);
    draw_text_array_320[l == 2] (txti[178], 8, 85, 0);
    draw_text_array_320[l == 3] (txti[179], 8, 106, 0);
    draw_text_array_320[l == 4] (txti[180], 8, 137, 0);
    draw_text_array_320[l == 5] (txti[181], 8, 158, 0);
    draw_text_array_320[l == 6] (txti[182], 227, 186, 0);
    sprintf (tmp2, "%d", xsize);
    draw_text_320 (tmp2, 248, 137, 1);
    sprintf (tmp2, "%d", ysize);
    draw_text_320 (tmp2, 248, 158, 1);
    pal2pal ((palette_ *) & tilesprev.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    memcpy (screen, corner[0], 64000);
    if (p >= 0)
      p--;
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
	img_free ((image_ *) & tilesprev);
	load_tile_set_preview (tiles, (image_ *) & tilesprev);
	for (i = 0; i < 52; i++)
	  memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
		  tilesprev.buffer + i * 62, 62);
	if (p == -1)
	  p = 0;
	event_sfx (111);
      } else if (l == 1) {
	l2 = t & 255;
	if (l2 >= 'a' && l2 <= 'z')
	  l2 -= 'a' - 'A';
	if ((l2 >= '0' && l2 <= '9') || (l2 >= '@' && l2 <= 'Z')
	    || (l2 >= '#' && l2 <= '&') || l2 == '!' || l2 == '_') {
	  if (pos < 8) {
	    tmp1[pos] = l2;
	    pos++;
	    tmp1[pos] = 0;
	    event_sfx (112);
	    flag = 1;
	  }
	}
	if ((t == HK_BackSpace || t == HK_Delete) && (pos > 0)) {
	  pos--;
	  tmp1[pos] = 0;
	  event_sfx (113);
	  flag = 1;
	}
	if (flag) {
	  sprintf(tmp2, "%s/%s.lvl", levels_output_dir, tmp1);
	  tmphdl = fopen (tmp2, "rb");
	  if (tmphdl != NULL) {
	    if (fread ((void *) &plinfo, sizeof (level_header_t), 1, tmphdl)
		== 1) {
	      xsize = BSWAP32 (plinfo.xt);
	      ysize = BSWAP32 (plinfo.yt);
	      xwrap = BSWAP32 (plinfo.xwrap);
	      ywrap = BSWAP32 (plinfo.ywrap);
	      tiles = atol ((char *) &(plinfo.tile_set_name[5])) - 1;
	      img_free ((image_ *) & tilesprev);
	      load_tile_set_preview (tiles, (image_ *) & tilesprev);
	      for (i = 0; i < 52; i++)
		memcpy (frmenu.buffer + 217 + 74 * 320 + i * 320,
			tilesprev.buffer + i * 62, 62);
	      if (p == -1)
		p = 0;
	      flaglock = 1;
	      event_sfx (117);
	    } else
	      flaglock = 0;
	    fclose (tmphdl);
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
    if (l == 6 && t == HK_Enter && tmp1[0] == 0)
      l = 1;
  } while (t != HK_Escape && !(l == 6 && t == HK_Enter));
  img_free (&frmenu);
  img_free (&tilesprev);
  if (l == 6 && t == HK_Enter) {
    event_sfx (116);

//          sprintf(tmp2,"%s %s %c %c %c %c",tmp1,tile_sets_names[tiles],xsize+' ',ysize+' ',((xwrap==-1)?'!':(xwrap+' ')),((ywrap==-1)?'!':(ywrap+' ')));
//          spawnl(P_WAIT,"HEDLITE.EXE","HEDLITE.EXE",tmp2,NULL);
    sprintf (tmp2, "%c %c %c %c", xsize + ' ', ysize + ' ',
	     ((xwrap == -1) ? '!' : (xwrap + ' ')),
	     ((ywrap == -1) ? '!' : (ywrap + ' ')));
    tmp2[1] = tmp2[3] = tmp2[5] = 0;

    hmain (7, tmp1, tile_sets_names[tiles], tmp2, tmp2 + 2, tmp2 + 4,
	   tmp2 + 6);
    /* update extra-levels list */
    free_extra_list ();
    browse_extra_directories ();
  }
  frame_old = frame_cur;
}

void
editor_first_menu (void)
{
  char l = 0;
  int t, j;
  if (extra_user_nbr == 0) {
    editor_menu ();
    return;
  }
  memset (pal.global, 63, 768);
  p = 64;

  do {
    background_menu ();
    j = minisinus[frame_cur & 31];
    draw_text_waving (txti[171], 159, 10, 1);
    draw_text_array[l == 0] (txti[172], 159, 85, 1);
    draw_text_array[l == 1] (txti[173], 159, 105, 1);
    copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		      corner[0] + j + (81 + l * 20) * xbuf + 50, 13, 20);
    copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		      corner[0] - j + (81 + l * 20) * xbuf + 320 - 50 - 13,
		      13, 20);
    pal2pal (&tile_set_img.palette, &pal, p);
    vsynch ();
    if (p >= 0)
      set_pal_with_luminance ((palette_rvb *) temppal.global);
    aff_buffer ();
    if (p == 0)
      p--;
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
draw_saved_games_info (int decal, char l, char h)
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
  copy_rect_transp (main_font_img.buffer + 134 + 50 * 320,
		    corner[0] + decal + (35 + l * 14) * xbuf + 1, 13, 20);
  copy_rect_transp (main_font_img.buffer + 121 + 50 * 320,
		    corner[0] + decal + (35 + l * 14) * xbuf + 320 - 1 - 13,
		    13, 20);
}
