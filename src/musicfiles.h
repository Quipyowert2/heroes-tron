/*------------------------------------------------------------------------.
| Copyright (C) 2000 Alexandre Duret-Lutz <duret_g@epita.fr>              |
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

/* Id: */

#ifndef __MUSICFILES_H__
#define __MUSICFILES_H__

typedef struct sound_track_s {
  char* alias;		/* name used in level files */
  char* filename;
  char* title;
  char* author;
} sound_track_t;

sound_track_t* sound_track_cons (char* alias, char* filename, 
				 char* title, char* author);
void sound_track_delete (sound_track_t* st);

sound_track_t* get_sound_track_from_alias (const char* alias);
void add_sound_track (sound_track_t* st);
void add_sound_track_cons (char* alias, char* filename, 
			   char* title, char* author);
int read_sound_config_file (char* filename);
int init_sound_track_list (void);

#endif
