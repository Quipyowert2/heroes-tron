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


/*
 *   Create the list of extra levels.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "extras.h"
#include "config.h"
#include "generic_list.h"
#include "hedlite.h"
#include "misc.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

typedef struct {
  filename_t	filename;
  char		is_in_user_dir;
} extradir_info_t;

NEW_LIST (extradir, extradir_info_t*);

extradir_list_t edir;

extra_level_t *extra_list = 0;
char *extra_selected_list = 0;
int extra_nbr = 0;

/* select only *.lvl files */
static int
select_file (const struct dirent *d)
{
  int l = strlen (d->d_name);
  return (l > 4 && d->d_name[l - 4] == '.' && d->d_name[l - 3] == 'l'
	  && d->d_name[l - 2] == 'v' && d->d_name[l - 1] == 'l');
}

/* compatr two filenames */
static int
cmp_filenames (const extra_level_t* l, const extra_level_t* r)
{
  return strcasecmp (l->level_name, r->level_name);
}

static void
browse_extra_directory (const char* directory, char is_in_user_dir)
{
  int i;
  struct dirent **tmp_list;
  int extra_nbr_here, old_nbr;

  /* get the files list of the directory */
  extra_nbr_here = scandir (directory, &tmp_list, select_file, alphasort);

  if (extra_nbr_here == -1) {
    fprintf (stderr, "extradir: ");
    perror (directory);
    return;
  }

  old_nbr = extra_nbr;
  extra_nbr += extra_nbr_here;
  /* realloc the list and the selection array */
  extra_selected_list = realloc (extra_selected_list, extra_nbr);
  memset (extra_selected_list, 0, extra_nbr);
  extra_list = realloc (extra_list, extra_nbr * sizeof (*extra_list));
  /* update the list */
  for (i = 0; i < extra_nbr_here; ++i) {
    char* fn = malloc (strlen (directory) + 1 + 
		       strlen(tmp_list[i]->d_name) + 1);
    extra_list[old_nbr + i].level_name = strdup (tmp_list[i]->d_name);
    sprintf(fn, "%s/%s", directory, tmp_list[i]->d_name);
    extra_list[old_nbr + i].full_name = fn;
    extra_list[old_nbr + i].is_in_user_dir = is_in_user_dir;
    strupr (extra_list[old_nbr + i].level_name);
    if ((fn = strchr (extra_list[old_nbr + i].level_name, '.')))
      *fn = 0;
    free (tmp_list[i]);
  }
  free (tmp_list);  
}

void
browse_extra_directories (void)
{
  extradir_list_t ed = edir;

  /* get the files of each directory */
  while (ed) {
    browse_extra_directory (ed->car->filename, ed->car->is_in_user_dir);
    ed = ed->cdr;
  }

  /* sort the files list */
  qsort (extra_list, extra_nbr, sizeof(*extra_list),
	 (int (*)(const void*,const void*))cmp_filenames);
}

void 
add_extra_directory (filename_t fn)
{
  extradir_info_t* tmp = malloc (sizeof (*tmp));
  tmp->filename = strdup (fn);
  tmp->is_in_user_dir = 0;
  extradir_push (&edir, tmp);
}

static void 
add_extra_in_user_directory (filename_t fn)
{
  extradir_info_t* tmp = malloc (sizeof (*tmp));
  tmp->filename = strdup (fn);
  tmp->is_in_user_dir = 1;
  extradir_push (&edir, tmp);
}

void
add_default_extra_directories (void)
{
  add_extra_directory (extradir);
  if (!create_levels_output_dir ())
    add_extra_in_user_directory (levels_output_dir);
}

void
free_extra_list (void)
{
  int i;

  for (i = 0; i < extra_nbr; ++i) {
    free (extra_list[i].full_name);
    free (extra_list[i].level_name);
  }
  extra_nbr = 0;
  free (extra_list);
  extra_list = 0;
  free (extra_selected_list);
  extra_selected_list = 0;
}
