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


#define USERCONFIG_FILE "heroesrc"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "userdir.h"
#include "misc.h"
#include "getshline.h"
#include "userconf.h"
#include "argv.h"
#include "config.h"
#include "extras.h"
#include "musicfiles.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

int 
read_userconf (const char* file, const char* argv0)
{
  char* filename;
  FILE* fs;
  int firstline = 0, endline = 0;
  char* buf = 0;
  size_t bufsize = 0;
#define MAX_ARGC 10
  
  if (!file)
    filename = strcat_alloc (userdir,"/" USERCONFIG_FILE);

  fs = fopen (filename, "r");

  if (!fs) {
    if (!file)
      free (filename);
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
	pname = malloc (sizeof(char) * (strlen (filename) + strlen (argv0) 
					+ strlen ("::999999") + 1));
	sprintf (pname, "%s:%s:%d", argv0, filename, firstline);
	argv[0] = pname;
	err = parse_argv (argc, argv);
	if (err)
	  return err;
      }
    } else if (!strcasecmp (argv[0], "setenv:")) {
      /* get the variable name */
      argv [1] = strtok (0, " \t\n");
      if (argv[1] == 0) {
	fprintf (stderr, "%s:%d: missing variable name\n", 
		 filename, firstline);	
	goto non_fatal_error;
      }
      argv[2] = strtok (0, "\n");
#ifdef DEBUG
      printf ("setenv(%s,%s)\n", argv[1], argv[2]);
#endif
      setenv (argv[1], argv[2], 1);
    } else if (!strcasecmp (argv[0], "extradir:")) {
      argv[1] = strtok (0, "\n");
      add_extra_directory (argv[1]);
    } else if (!strcasecmp (argv[0], "soundconf:")) {
      argv[1] = strtok (0, "\n");
      read_sound_config_file (argv[1]);      
    } else {
      fprintf (stderr, "%s:%d: unknown keyword `%s'\n", 
	       filename, firstline, argv[0]);
      if (!file)
	free (filename);
      return 1;
    }
  non_fatal_error:
    ;
  } 
  free (buf);
  fclose (fs);
  if (!file)
    free (filename);
  return 0;
}
