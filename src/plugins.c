/*------------------------------------------------------------------.
| Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License version 2 as    |
| published by the Free Software Foundation.                        |
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
#include "plugins.h"
#include "ltdl.h"
#include "rsc_files.h"
#include "errors.h"

void
plugins_initialize (void)
{
  char *name = get_rsc_file ("plug-in-dir");
  int err = lt_dlinit ();
  if (err)
    emsg (_("libltdl:lt_dlinit reported %d errors"), err);
  lt_dladdsearchdir (name);
  free (name);
}

void
plugins_finalize (void)
{
  int err = lt_dlexit ();
  if (err)
    emsg (_("libltdl:lt_dlexit reported %d errors"), err);
}

void
plugin_load (const char *name)
{
  void (*initialize)(void);
  lt_dlhandle hdl = lt_dlopenext (name);
  if (!hdl)
    emsg (_("failed to dlopen plug-in `%s'"), name);
  initialize = (void (*)(void)) lt_dlsym (hdl, "initialize");
  if (!initialize)
    emsg (_("no `initialize' symbol in plug-in `%s'"), name);

  initialize();
}

void
plugin_unload (const char *name)
{
  (void) name;
}
