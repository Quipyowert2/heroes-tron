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

/* this file will be obsolete once gettext is used */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#include <ctype.h>
#include "errors.h"
#include "txts.h"
#include "rsc_files.h"
#include "debugmsg.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

static unsigned max_txti;
char **txti = NULL;

static void
remove_comments (char *str)
{
  int i;
  for (i = 0;
       (str[i] != ';') && (str[i] != 0) && (str[i] != 13) && (str[i] != 10);
       i++);
  i--;
  for (; str[i] == ' '; i--);
  str[i + 1] = 0;
}

/*static char* forward_spaces(char*tmp) {
   while (*tmp==' ') tmp++;
   return (tmp);
} */

extern void
read_txti_cfg (void)
{
  FILE *fconf;
  char tmp[256];
  char *tmpptr = (char *) tmp;
  char *tmpptr2;
  char c;
  unsigned int nbr;
  char *t = get_non_null_rsc_file ("text-conf-txt");

  dmsg (D_SECTION|D_FILE, "parsing text configuration file: %s ...", t);
  if ((fconf = fopen (t, "rt")) == NULL)
    fatal_error ("Can't read text configuration file.\n");
  free (t);
  while (fgets (tmpptr, 256, fconf) != NULL) {
    c = toupper (tmp[0]);
    remove_comments (tmpptr);
    if (c == 'M') {
      if (txti != NULL)
	return;
      max_txti = atol (tmpptr + 1);
      txti = (char **) malloc (max_txti * sizeof (char *));
      memset (txti, 0, max_txti * sizeof (char *));
    } else if (c == 'T') {
      tmpptr2 = strchr (tmpptr, ' ');
      *tmpptr2++ = 0;
      nbr = atol (tmpptr + 1);
//            tmpptr2=forward_spaces(tmpptr2);
//            printf("|%s|\n",tmpptr2);
      if (nbr < max_txti)
	txti[nbr] = strdup (tmpptr2);
    }
  }
  fclose (fconf);
  dmsg (D_SECTION|D_FILE, "... done.");
}

void
close_txti (void)
{
  unsigned int i;

  dmsg (D_MISC, "free txti");

  for (i = 0; i < max_txti; ++i)
    if (txti[i])
      free (txti[i]);
  free (txti);
}
