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
#include "relocate.h"
#include "debugmsg.h"
#include "errors.h"
#include "isdir.h"
#include "rsc_files.h"

static void
check_localedir_env (void)
{
  char* locale_dir;
  dmsg (D_SYSTEM, "looking for HEROES_LOCALE_DIR or HEROES_LOCALEDIR...");
  if ((locale_dir = getenv ("HEROES_LOCALE_DIR")) ||
      (locale_dir = getenv ("HEROES_LOCALEDIR"))) {
    dmsg (D_SYSTEM,"... found: %s", locale_dir);
    set_rsc_file ("locale-dir", locale_dir, false);
  } else {
    dmsg (D_SYSTEM, "... not found.");
  }
}

static void
check_datadir_env (void)
{
  char* data_dir;
  dmsg (D_SYSTEM, "looking for HEROES_DATA_DIR or HEROES_DATADIR...");
  if ((data_dir = getenv ("HEROES_DATA_DIR")) ||
      (data_dir = getenv ("HEROES_DATADIR"))) {
    dmsg (D_SYSTEM,"... found: %s", data_dir);
    set_rsc_file ("data-dir", data_dir, false);
  } else {
    dmsg (D_SYSTEM, "... not found.");
  }
}

static void
check_homedir_env (void)
{
  char* home_dir;
  dmsg (D_SYSTEM, "looking for HEROES_HOME_DIR, HEROES_HOMEDIR or HOME...");
  if ((home_dir = getenv ("HEROES_HOME_DIR")) ||
      (home_dir = getenv ("HEROES_HOMEDIR")) ||
      (home_dir = getenv ("HOME"))) {
    dmsg (D_SYSTEM,"... found: %s", home_dir);
    set_rsc_file ("home-dir", home_dir, false);
  } else {
    dmsg (D_SYSTEM, "... not found.");
    wmsg (_("HOME variable not found in environment, defaulting to `.'"));
    set_rsc_file ("home-dir", ".", false);
  }
}

static bool
check_prefix_env (void)
{
  char* prefix;
  dmsg (D_SYSTEM, "looking for HEROES_PREFIX ...");
  if ((prefix = getenv ("HEROES_PREFIX"))) {
    dmsg (D_SYSTEM,"... found: %s", prefix);
    set_rsc_file ("prefix", prefix, false);
    return true;
  } else {
    dmsg (D_SYSTEM, "... not found.");
    return false;
  }
}

/* Check whether the installation looks correct.  */
static bool
check_installation (void)
{
  char *dir = get_non_null_rsc_file ("levels-dir");
  bool ok = isdir (dir);
  dmsg (D_SYSTEM,
	ok ? "directory %s found": "%s absent or not a directory",
	dir);
  free (dir);
  return ok;
}


bool
relocate_data (void)
{
  /* Check whether the user has set some environment variables to
     override internal paths.  */
  check_datadir_env ();
  check_localedir_env ();
  check_homedir_env ();
  if (!check_prefix_env ()) {
    if (!check_installation ()) {
      /* BACKWARD_RELATIVE_BINDIR is in case the binary has been
	 run from the current directory.  */
      dmsg (D_SYSTEM, "default prefix looks wrong, trying '%s'",
	    BACKWARD_RELATIVE_BINDIR);
      set_rsc_file ("prefix", BACKWARD_RELATIVE_BINDIR, false);
      if (!check_installation ())
	emsg (_("\
It looks like the game is not correctly installed.\n\
Maybe the data files has not been installed with the same configure options\n\
as the executable, or maybe the data files have been moved elsewhere.\n\
In the latter case it's probably enough to set the environment variable\n\
HEROES_PREFIX to the new location.  You may also want to set\n\
HEROES_DEBUG=system to see what files Heroes is looking after.\n\
If none of this helps, contact <heroes-bugs@lists.sourceforge.net>\n"));
    }
  }
  return false;
}
