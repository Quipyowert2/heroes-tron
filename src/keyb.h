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


#ifndef _KEYB_H_plx_
#define _KEYB_H_plx_

#define KEY_MAX 0xffff
extern unsigned char keyboard_map[KEY_MAX + 1];
extern unsigned int keyboard_modifiers;

void init_keyboard_map (void);
void uninit_keyboard_map (void);
void process_input_events (void);
int get_key (void);
int key_ready (void);

int init_mouse (void);
void mouse_show (void);
void mouse_hide (void);
int mouse_x (void);
int mouse_y (void);
char mouse1 (void);
char mouse2 (void);
char mouse12 (void);
/* 
char mouse3 (void);
char mouse123 (void);
void set_mouse_pos (int, int); 
*/

#endif
