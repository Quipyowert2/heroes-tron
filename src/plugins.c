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

/* For some unclear reason, ltdl.h define this symbol the the
   LTDL_SET_PRELOADED_SYMBOLS macro only.  */
extern const lt_dlsymlist lt_preloaded_symbols[];

void
plugins_initialize (void)
{
  char *name = get_rsc_file ("plug-in-dir");
  int err;

  err = lt_dlpreload_default (lt_preloaded_symbols);
  if (err)
    emsg (_("%s reported %d errors"), "lt_dlpreload_default",  err);

  err = lt_dlinit ();
  if (err)
    emsg (_("%s reported %d errors"), "lt_dlinit",  err);

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
