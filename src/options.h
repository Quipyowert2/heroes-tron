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

/* chargement, et reinitialisation des options d'Heroes */

#ifndef HEROES__OPTIONS__H
#define HEROES__OPTIONS__H

typedef struct
{
  /* screen in_menu options */
  u8_t screen_size;		/* 0=max */
  u8_t radar_map;
  u8_t _obsolete;		/* was use_glenz */
  u8_t display_infos;
  u8_t luminance;
  u8_t inertia;
  /* music in_menu options */
  u8_t music;
  u8_t music_volume;		/* 0=max */
  u8_t sfx;
  u8_t sfx_volume;		/* 0=max */
  /* control in_menu options */
  u8_t ctrl_one;		/* 0=keyboard */
  u8_t ctrl_two;		/* 1=joystick */
  u8_t autopilot_one;
  u8_t autopilot_two;
  /* game in_menu options */
  u8_t ghosts;
  u8_t speed;
  u32_t gamerounds;
  u32_t player_color[4];
  /* keyboard in_menu options */
  u32_t player_keys[2][6];	/* FIXME: should be of type keycode_t,
				   but the size of keycode_t is defined
				   by the library used to read the keys. */
  /* extra in_menu options */
  u8_t extras;
}
ATTRIBUTE_PACKED options_t;

extern options_t opt;
extern char extrasel;
void load_options (void);
void write_options (void);
void reinit_options (void);
void free_options (void);

#endif /* HEROES__OPTIONS__H */
