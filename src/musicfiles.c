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

#include <string.h>
#include <stdlib.h>
#include "musicfiles.h"
#include "generic_list.h"
#include "config.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

NEW_LIST(st,sound_track_t*);

static st_list_t sound_track_list;

sound_track_t* 
sound_track_cons (char* alias, char* filename, char* title, char* author)
{
  sound_track_t* st = malloc (sizeof (*st));
  st->alias = strdup (alias);
  st->filename = strdup (filename);
  st->title = strdup (title);
  st->author = strdup (author);
  return st;
}

void 
sound_track_delete (sound_track_t* st)
{
  free (st->alias);
  free (st->filename);
  free (st->title);
  free (st->author);
  free (st);
}

void 
add_sound_track (sound_track_t* st)
{
  st_push(&sound_track_list, st);
}

void 
add_sound_track_cons (char* alias, char* filename, char* title, char* author)
{
  add_sound_track (sound_track_cons (alias, filename, title, author));
}

sound_track_t* 
get_sound_track_from_alias (const char* alias)
{
  st_list_t list = sound_track_list;

  while (sound_track_list) {
    if (!strcasecmp (list->car->alias, alias))
      return list->car;
    list = list->cdr;
  }
  return 0;
}

int 
read_config_file (const char* filename __attribute__ ((unused)))
{
  return 0;
}

int 
init_sound_track_list (void)
{
  add_sound_track_cons ("HEROES01", moddir "heroes01.xm", 
			"Corridor 1", "Alexel");
  add_sound_track_cons ("HEROES02", moddir "heroes02.xm", 
			"Caero", "Tnk");
  add_sound_track_cons ("HEROES03", moddir "heroes03.xm", 
			"Dark Ages", "Alexel");
  add_sound_track_cons ("HEROES04", moddir "heroes04.xm", 
			"Corridor 2", "Alexel");
  add_sound_track_cons ("HEROES05", moddir "heroes05.xm", 
			"Vulcano", "Tnk");
  add_sound_track_cons ("HEROES06", moddir "heroes06.xm", 
			"Electric Dream", "Tnk");
  add_sound_track_cons ("HEROES07", moddir "heroes07.xm", 
			"Metal Master", "Tnk");
  add_sound_track_cons ("HEROES08", moddir "heroes08.xm", 
			"Moon 51", "Tnk");
  add_sound_track_cons ("HEROES09", moddir "heroes09.xm", 
			"Corridor 3", "Alexel");
  add_sound_track_cons ("HEROES10", moddir "heroes10.xm", 
			"Sweet Dream", "Alexel");
  add_sound_track_cons ("INTRO", moddir "intro.xm", 
			"Heroes Intro", "Alexel");
  add_sound_track_cons ("ENDSCROLL", moddir "sdf.xm", 
			"Heroes End Scroll", "Alexel");
  add_sound_track_cons ("MENU", moddir "menu.xm", 
			"Heroes Menu", "Alexel");
  return 0;
}
