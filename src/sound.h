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

#ifndef HEROES__SOUND__H
#define HEROES__SOUND__H

#include "sprite.h"

extern char* soundtrack_author;
extern char* soundtrack_title;
extern sprite_t* soundtrack_author_sprite;
extern sprite_t* soundtrack_title_sprite;

#if (HAVE_LIBSDL_MIXER || HAVE_LIBMIKMOD)

void set_volume (void);
void halve_volume (void);
int init_sound_engine (void);
void uninit_sound_engine (void);
void unload_soundtrack (void);
void play_soundtrack (void);
void print_drivers_list (void);
void decode_sound_options (char* option_string, char* argv0);

void load_soundtrack_from_alias (const char* alias);
void load_next_soundtrack (void);
void load_prev_soundtrack (void);

#else /* !HAVE_LIBSDL_MIXER && !HAVE_LIBMIKMOD */

#define set_volume()
#define halve_volume()
#define init_sound_engine() 0
#define uninit_sound_engine()
#define unload_soundtrack()
#define play_soundtrack()
void print_drivers_list (void);
#define decode_sound_options(x,y)
#define load_soundtrack_from_alias(x)
#define load_next_soundtrack()
#define load_prev_soundtrack()

#endif /* !HAVE_LIBSDL_MIXER && !HAVE_LIBMIKMOD */

#endif /* HEROES__SOUND__H */
