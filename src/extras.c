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

#include "common.h"
#include "extras.h"
#include "generic_list.h"
#include "hedlite.h"
#include "misc.h"
#include "rsc_files.h"
#include "debugmsg.h"

typedef struct {
  filename_t	filename;
  char		is_in_user_dir;	/* Is this extra level a user level?
				 (user levels come from the ~/.heroes/level/
				 directory */
} extradir_info_t;

NEW_LIST (extradir, extradir_info_t*);

extradir_list_t edir;

int extra_nbr = 0;		/* The total number of extra levels */
int extra_user_nbr = 0;		/* The number of user levels from */

extra_level_t *extra_list = 0;	/* The list of extra-levels, the user's
				   extra-levels are at the beginning */
char *extra_selected_list = 0;	/* For each level: 1 if selected, 0 if not */

/* select only *.lvl files */
static int
select_file (const struct dirent *d)
{
  int l = strlen (d->d_name);
  return (l > 4 && d->d_name[l - 4] == '.' && d->d_name[l - 3] == 'l'
	  && d->d_name[l - 2] == 'v' && d->d_name[l - 1] == 'l');
}

/* compare two extra-levels for sorting,
   we want to sort user's levels first, and then alphabeticaly */
static int
cmp_extralevels (const extra_level_t* l, const extra_level_t* r)
{
  int d = r->is_in_user_dir - l->is_in_user_dir;

  if (d != 0)
    return d;
  return strcasecmp (l->level_name, r->level_name);
}

#ifndef HAVE_ALPHASORT
int alphasort (const struct dirent **a, const struct dirent **b);
#endif

#ifndef HAVE_SCANDIR
int scandir (const char *dir, struct dirent ***namelist,
	     int (*select)(const struct dirent *),
	     int (*compar)(const struct dirent **, const struct dirent **));
#endif

static void
browse_extra_directory (const char* directory, char is_in_user_dir)
{
  int i;
  struct dirent **tmp_list;
  int extra_nbr_here, old_nbr;

  dmsg (D_FILE, "browsing directory %s ...", directory);
  /* get the files list of the directory */
  extra_nbr_here = scandir (directory, &tmp_list, select_file, alphasort);

  if (extra_nbr_here == -1) {
    if (is_in_user_dir) 
      perror (directory);
    dperror ("scandir");
    return;
  }

  dmsg (D_FILE, "... %d files", extra_nbr_here);

  if (extra_nbr_here == 0)
    return;

  old_nbr = extra_nbr;
  extra_nbr += extra_nbr_here;
  if (is_in_user_dir)
    extra_user_nbr += extra_nbr_here;
  /* realloc the list and the selection array */
  extra_selected_list = realloc (extra_selected_list, extra_nbr);
  memset (extra_selected_list, 0, extra_nbr);
  extra_list = realloc (extra_list, extra_nbr * sizeof (*extra_list));
  /* update the list */
  for (i = 0; i < extra_nbr_here; ++i) {
    char* fn = malloc (strlen (directory) + 1 + 
		       strlen(tmp_list[i]->d_name) + 1);
#if !defined HAVE_SCANDIR && defined D_NAME_IS_POINTER
    /* d_name has been allocated by the scandir replacement */
    extra_list[old_nbr + i].level_name = tmp_list[i]->d_name;
#else
    extra_list[old_nbr + i].level_name = strdup (tmp_list[i]->d_name);
#endif
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
	 (int (*)(const void*,const void*))cmp_extralevels);
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
  char* t;
  dmsg (D_SECTION, "setup default extra directory");
  t = get_rsc_file ("extra-levels-dir");
  if (t) {
    add_extra_directory (t);
    free (t);
  }
  if (!create_levels_output_dir ())
    add_extra_in_user_directory (levels_output_dir);
}

void
free_extra_list (void)
{
  int i;

  if (extra_nbr == 0)
    return;

  dmsg (D_MISC, "freeing extra list");

  for (i = 0; i < extra_nbr; ++i) {
    free (extra_list[i].full_name);
    free (extra_list[i].level_name);
  }
  extra_nbr = 0;
  extra_user_nbr = 0;
  free (extra_list);
  extra_list = 0;
  free (extra_selected_list);
  extra_selected_list = 0;
}

void
free_extra_directories (void)
{
  extradir_list_t next;

  dmsg (D_MISC, "free extra directories");

  while (edir) {
    next = edir->cdr;
    free (edir->car->filename);
    free (edir->car);
    free (edir);
    edir = next;
  }

  free_levels_output_dir ();
}
