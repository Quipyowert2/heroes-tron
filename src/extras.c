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
				 directory) */
} extradir_info_t;

static void free_extradir_info (extradir_info_t* ei);

NEW_LIST (extradir, extradir_info_t*, STD_EQUAL, free_extradir_info);

/* level_list_t is used for building a temporaly list of all extra
   levels seen in directories.  The list will be converted to an
   array, so though the data are pointer we don't want to free them
   when cleaning the list with in level_clear */
NEW_LIST (level, extra_level_t*, STD_EQUAL, NULL_DESTRUCTOR);

extradir_list_t edir;

int extra_nbr = 0;		/* The total number of extra levels */
int extra_user_nbr = 0;		/* The number of user levels from */

extra_level_t *extra_list = 0;	/* The list of extra-levels, the user's
				   extra-levels are at the beginning */
char *extra_selected_list = 0;	/* For each level: 1 if selected, 0 if not */


static void free_extradir_info (extradir_info_t* ei)
{
  free (ei->filename);
  free (ei);  
}

/* select only *.lvl files */
static int
select_file (const struct dirent *d)
{
  int l = strlen (d->d_name);
  return (l > 4 && d->d_name[l - 4] == '.' && d->d_name[l - 3] == 'l'
	  && d->d_name[l - 2] == 'v' && d->d_name[l - 1] == 'l');
}

/* compare two extra-levels for sorting,
   we want to sort user's levels first, and then alphabetically */
static int
cmp_extralevels (const extra_level_t* l, const extra_level_t* r)
{
  int d = r->is_in_user_dir - l->is_in_user_dir;

  if (d != 0)
    return d;
  return strcasecmp (l->level_name, r->level_name);
}

/* Browse a directory, adds the levels found to ll.
   Update extra_nbr and extra_user_nbr. */
static void
browse_extra_directory (const char* directory, char is_in_user_dir, 
			level_list_t* ll)
{
  DIR* dir;
  struct dirent* de;
  int n = 0;

  dmsg (D_FILE, "browsing directory %s ...", directory);

  dir = opendir (directory);
  if (!dir) {
    /* Always output an error message when handling a user directory.
       If not, we are probably reading a default extra directory,
       maybe system wide configured or hard coded in the source;
       it's best to assume this is not an error (this allow the
       addition of extra levels as packages or such).  */
    if (is_in_user_dir) 
      perror (directory);
    dperror ("scandir");
    return;
  }
    
  while ((de = readdir (dir)))
    if (select_file (de)) {
      /* add the file to the list */
      extra_level_t* tmp = malloc (sizeof (*tmp));
      char* fn = malloc (strlen (directory) + 1 + 
			 strlen (de->d_name) + 1);

      tmp->level_name = strdup (de->d_name);
      sprintf (fn, "%s/%s", directory, tmp->level_name);
      tmp->full_name = fn;
      tmp->is_in_user_dir = is_in_user_dir;

      strupr (tmp->level_name);
      if ((fn = strchr (tmp->level_name, '.')))
	*fn = 0;

      level_push (ll, tmp);
      ++n;
    }

  closedir (dir);

  dmsg (D_FILE, "... %d files", n);

  extra_nbr +=n;
  if (is_in_user_dir)
    extra_user_nbr += n;
}

void
browse_extra_directories (void)
{
  extradir_list_t ed = edir;
  level_list_t ll = 0;
  level_list_t ll_cur;
  int i;

  /* build the list of the files found in each directory */
  while (ed) {
    browse_extra_directory (ed->car->filename, ed->car->is_in_user_dir, &ll);
    ed = ed->cdr;
  }

  /* convert the list to an array (this array is called extra_list, BTW :)) */
  extra_list = realloc (extra_list, extra_nbr * sizeof (*extra_list));
  for (i = 0, ll_cur = ll; ll_cur; ll_cur = ll_cur->cdr, ++i)
    extra_list[i] = *(ll_cur->car);
  assert (i == extra_nbr);

  /* ll is now useless */
  level_clear (&ll);

  /* sort the files list */
  qsort (extra_list, extra_nbr, sizeof(*extra_list),
	 (int (*)(const void*,const void*))cmp_extralevels);

  /* allocate the selection array */
  extra_selected_list = realloc (extra_selected_list, extra_nbr);
  memset (extra_selected_list, 0, extra_nbr);
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
  dmsg (D_MISC, "free extra directories");
  extradir_clear (&edir);

  free_levels_output_dir ();
}
