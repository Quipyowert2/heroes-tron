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

#include "common.h"
#include "getshline.h"
#include "musicfiles.h"
#include "generic_list.h"
#include "misc.h"
#include "rsc_files.h"
#include "debugmsg.h"

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

  while (list) {
    if (!strcasecmp (list->car->alias, alias))
      return list->car;
    list = list->cdr;
  }
  return 0;
}

static char* 
dir_name (const char* filename)
{
  char* pos = strrchr (filename, '/');
  char* res;
  if (pos == 0)
    return 0;
  res = malloc (pos - filename + 2);
  strncpy (res, filename, pos - filename + 1);
  res[pos - filename + 1] = 0;
  return res;
}

int 
read_sound_config_file (char* filename)
{
  FILE* fs;
  char* buf = 0;
  size_t bufsize = 0;
  int firstline = 0, endline = 0;
  char* expfilename = rsc_expand (filename);
  char* dir = dir_name (expfilename);

  dmsg (D_SECTION|D_FILE,"reading sound config file: %s ...", expfilename);

  fs = fopen (expfilename, "r");

  if (!fs) {
    dmsg (D_SECTION|D_FILE,"... could not open.");
    dperror ("fopen");
    free (expfilename);
    free (dir);
    return 0;
  }

  while (getshline_numbered 
	 (&firstline, &endline, &buf, &bufsize, fs) != -1) {
    char* alias = strtok (buf, ":\n");
    char* file  = strtok (0, ":\n");
    char* title  = strtok (0, ":\n");
    char* author  = strtok (0, "\n");    
    if (!alias || !alias[0])
      fprintf (stderr, "%s:%d: missing alias name\n", 
	       filename, firstline);	
    else if (!file || !file[0])
      fprintf (stderr, "%s:%d: missing file name\n", 
	       filename, firstline);	
    else if (!title || !title[0])
      fprintf (stderr, "%s:%d: missing title\n", 
	       filename, firstline);	
    else if (!author || !author[0])
      fprintf (stderr, "%s:%d: missing author\n", 
	       filename, firstline);	
    else {
      if (dir && file[0] != '/') {
	char* tmp = strcat_alloc (dir, file);
	add_sound_track_cons (alias, tmp, title, author);
	free (tmp);
      } else
	add_sound_track_cons (alias, file, title, author);
    }
  }
  fclose (fs);
  free (buf);
  free (dir);
  free (expfilename);
  dmsg (D_SECTION|D_FILE,"... done.");

  return 0;
}

int 
init_sound_track_list (void)
{
  /* No soundtrack by default */
  /*
  add_sound_track_cons ("MENU", moddir "menu.xm", 
			"Heroes Menu", "Alexel");
	...
  */
  return 0;
}

void 
uninit_sound_track_list (void)
{
  st_list_t next;

  dmsg (D_MISC, "free sound track list");

  while (sound_track_list) {
    next = sound_track_list->cdr;
    sound_track_delete (sound_track_list->car);
    free (sound_track_list);
    sound_track_list = next;
  }
}
