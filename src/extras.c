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
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

char **extra_list = 0;

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
cmp_filenames (const char** l, const char** r)
{
  return strcasecmp (*l, *r);
}

void
browse_extra_directory (const char* directory)
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
    extra_list[old_nbr + i] = strdup (tmp_list[i]->d_name);
    free (tmp_list[i]);
  }
  free (tmp_list);  

  /* sort the list */
  qsort (extra_list, extra_nbr, sizeof(*extra_list), 
	 (int (*)(const void*,const void*))cmp_filenames);
}

void
make_extra_list (void)
{
  browse_extra_directory (extradir);
}
