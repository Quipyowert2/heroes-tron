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

/* chargement, et reinitialisation des options d'Heroes */

#ifndef HEROES__OPTIONS__H
#define HEROES__OPTIONS__H

typedef struct
{
  /* screen in_menu options */
  char screen_size;		// 0=max
  char radar_map;
  char use_glenz;
  char display_infos;
  char luminance;
  char inertia;
  /* music in_menu options */
  char music;
  char music_volume;		// 0=max
  char sfx;
  char sfx_volume;		// 0=max
  /* control in_menu options */
  char ctrl_one;		// 0=keyboard
  char ctrl_two;		// 1=joystick
  char autopilot_one;
  char autopilot_two;
  /* game in_menu options */
  char ghosts;
  char speed;
  int gamerounds;
  int player_color[4];
  /* keyboard in_menu options */
  int player_keys[2][6];
  /* extra in_menu options */
  char extras;
}
options_t __attribute__ ((packed));

extern options_t opt;
extern char extrasel;
void load_options (void);
void write_options (void);
void reinit_options (void);
void free_options (void);

#endif /* HEROES__OPTIONS__H */
