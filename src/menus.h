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


#ifndef __MENUS_H_plx__
#define __MENUS_H_plx__

void background_menu (void);
/*
void control_menu(void);
void keyboard_menu(void);
void sound_menu(void);
void screen_menu(void);
void game_menu(void);
*/
void demo_info (void);
void option_menu (void);
char quit_menu (void);
void draw_play_menu (char l);

void draw_main_menu (char l);
void draw_quit_menu (char l);
void editor_first_menu (void);

void draw_saved_games_info (int decal, char l, char h);

#endif
