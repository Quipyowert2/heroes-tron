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

#include "config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "userdir.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

#define DIR_NAME ".heroes"

char* userdir;

/* Test if a file exists and is a directory */
int 
exists_dir (char* dir) 
{
  struct stat s;
  int err = stat (dir, &s);
  if (err) {
    if (errno == ENOENT) 
      return 0;
    fprintf(stderr,"%d\n",err);
    perror (dir);
    return -1;
  }
  if (!S_ISDIR(s.st_mode)) {
    fprintf (stderr, "%s is not a directory.\n", dir);
    return -1;
  }
  return 1;
}


/* expand ~/.heroes and create that directory, if needed */
int 
setup_userdir (void)
{
  char* home = getenv ("HOME");

  if (!home) {
    fprintf (stderr, "No $HOME found in environment.\n");
    return 1;
  }
  userdir = malloc (strlen (home) + 1 + sizeof (DIR_NAME));
  if (!userdir) {
    fprintf (stderr, "Not enough memory.\n");
    return 1;
  }
  sprintf(userdir, "%s/" DIR_NAME, home);

  {
    int err = exists_dir (userdir);

    if (err < 0)
      return 1;
    if (err == 0) {
      if (mkdir (userdir
#if TWO_ARGS_MKDIR
		 , 0700
#endif
		 )) {
	perror ("while creating ~/" DIR_NAME);
	return 1;
      } else {
	fprintf(stderr, "directory ~/" DIR_NAME " created.\n");      
      }
    }
  }
  return 0;
}
