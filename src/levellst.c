/*------------------------------------------------------------------.
| Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License as published by |
| the Free Software Foundation; either version 2 of the License, or |
| (at your option) any later version.                               |
|                                                                   |
| Heroes is distributed in the hope that it will be useful, but     |
| WITHOUT ANY WARRANTY; without even the implied warranty of        |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU |
| General Public License for more details.                          |
|                                                                   |
| You should have received a copy of the GNU General Public License |
| along with this program; if not, write to the Free Software       |
| Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          |
| 02111-1307 USA                                                    |
`------------------------------------------------------------------*/

#include "system.h"
#include "levellst.h"
#include "lvl.h"
#include "debugmsg.h"
#include "errors.h"
#include "rsc_files.h"

/* select only *.lvl files */
int
select_file_lvl (const struct dirent *d)
{
  int l = strlen (d->d_name);
  return (l > 4 && d->d_name[l - 4] == '.' && d->d_name[l - 3] == 'l'
	  && d->d_name[l - 2] == 'v' && d->d_name[l - 1] == 'l');
}

level_info_t *level_list = 0;
size_t level_list_size = 0;
static size_t level_list_max = 0;

static void
read_level_dir (const char *dirname)
{
  DIR *dir;
  struct dirent* de;
  int n = 0;

  dmsg (D_FILE | D_SECTION, "reading level list from %s", dirname);

  dir = opendir (dirname);
  if (!dir) {
    dperror ("opendir");
    emsg (_("cannot open directory %s"), dirname);
  }

  while ((de = readdir (dir)))
    if (select_file_lvl (de)) {
      char *filename;
      level_t tmp_lvl;

      if (level_list_size >= level_list_max) {
	level_list_max += 32;
	XREALLOC_ARRAY (level_list, level_list_max);
      }

      filename = xmalloc (strlen (dirname) + 1 + strlen (de->d_name) + 1);
      sprintf (filename, "%s/%s", dirname, de->d_name);
      level_list[level_list_size].name = filename;

      dmsg (D_FILE, "loading header from %s", filename);
      lvl_load_file (filename, &tmp_lvl, false);
      level_list[level_list_size].wrapped =
	(tmp_lvl.tile_width_wrap != DONT_WRAP
	 && tmp_lvl.tile_height_wrap != DONT_WRAP);
      /* FIXME: lvl_free (&tmp_lvl);  */

      ++level_list_size;
    }

  closedir (dir);
  dmsg (D_FILE, "... %d files", n);
}

int
read_level_list (void)
{
  char *dirname = get_non_null_rsc_file ("levels-dir");
  read_level_dir (dirname);
  free (dirname);
  return 0;
}

void
free_level_list (void)
{
  size_t i;
  for (i = 0; i < level_list_size; ++i)
    free (level_list[i].name);
  free (level_list);
  level_list = 0;
  level_list_max = 0;
  level_list_size = 0;
}
