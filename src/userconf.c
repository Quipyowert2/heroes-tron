/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
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
#include "userdir.h"
#include "misc.h"
#include "getshline.h"
#include "userconf.h"
#include "argv.h"
#include "extras.h"
#include "musicfiles.h"
#include "rsc_files.h"
#include "debugmsg.h"
#include "errors.h"

int 
read_userconf (const char* file, const char* argv0)
{
  FILE* fs;
  int firstline = 0, endline = 0;
  char* buf = 0;
  size_t bufsize = 0;
#define MAX_ARGC 10
  
  dmsg (D_SECTION|D_FILE, "reading configuration file: %s ...", file);

  fs = fopen (file, "r");

  if (!fs) {
    dmsg (D_SECTION|D_FILE, "... could not open.");
    dperror ("fopen");
    return 0;
  }

  while (getshline_numbered 
	 (&firstline, &endline, &buf, &bufsize, fs) != -1) {
    int argc;
    char* argv[MAX_ARGC];

    argv[0] = strtok (buf, " \t\n");
    if (!strcasecmp (argv[0], "Options:")) {
      /* fetch the options */
      argc = 1;
      while (argc < MAX_ARGC) {
	argv [argc] = strtok (0, " \t\n");
	if (argv[argc] == 0)
	  break;
	else
	  ++argc;
      }
      /* process the options */
      {
	char* pname;
	int err;
	pname = malloc (sizeof(char) * (strlen (file) + strlen (argv0) 
					+ strlen ("::999999") + 1));
	sprintf (pname, "%s:%s:%d", argv0, file, firstline);
	argv[0] = pname;
	err = parse_argv (argc, argv);
	free (pname);
	if (err)
	  return err;
      }
    } else if (!strcasecmp (argv[0], "setenv:")) {
      char *s;
      /* get the variable name */
      argv [1] = strtok (0, " \t\n");
      if (argv[1] == 0) {
	wmsg ("%s:%d: missing variable name\n", file, firstline);	
	goto non_fatal_error;
      }
      argv[2] = strtok (0, "\n");
      dmsg (D_SYSTEM, "setenv(%s,%s)", argv[1], argv[2]);
      s = malloc (strlen (argv[1]) + strlen (argv[2]) + 2);
      sprintf (s, "%s=%s", argv[1], argv[2]);
      putenv (s);
      free (s);
    } else if (!strcasecmp (argv[0], "extradir:")) {
      argv[1] = strtok (0, "\n");
      add_extra_directory (argv[1]);
    } else if (!strcasecmp (argv[0], "soundconf:")) {
      argv[1] = strtok (0, "\n");
      read_sound_config_file (argv[1]);
    } else if (!strcasecmp (argv[0], "setrsc:")){
      /* get the resource name */
      argv [1] = strtok (0, " \t\n");
      if (argv[1] == 0) {
	wmsg ("%s:%d: missing resource name\n", file, firstline);
	goto non_fatal_error;
      }
      argv[2] = strtok (0, "\n");
      set_rsc_file (argv[1], argv[2]);
    } else {
      wmsg ("%s:%d: unknown keyword `%s'\n", file, firstline, argv[0]);
      return 1;
    }
  non_fatal_error:
    ;
  } 
  free (buf);
  fclose (fs);
  dmsg (D_SECTION|D_FILE, "... finished reading %s.", file);
  return 0;
}
