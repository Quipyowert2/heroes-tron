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

#ifndef HEROES__MENUS__H
#define HEROES__MENUS__H

#include "sprite.h"

void init_menus_sprites (void);
void uninit_menus_sprites (void);
void waving_arrows (unsigned int row, unsigned int col);


void background_menu (void);

void option_menu (void);
char quit_menu (void);
void draw_play_menu (int l);

void draw_main_menu (int l);
void draw_quit_menu (int l);
void editor_first_menu (void);

void draw_saved_games_info (int decal, int l, char h);

void jukebox_menu (void);
void jukebox_draw (int pos);
int jukebox_keys (int *pos);

#endif /* HEROES__MENUS__H */
