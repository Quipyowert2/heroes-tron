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


/* sauvegarde des scores d'Heroes */

#include "structs.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "scores.h"
#include "config.h"
#include "misc.h"
#include "userdir.h"
#include "endian.h"

top_score highs[5][10];

#define SCORES_FILE "scores.dat"
#if USER_SCORES_FILE
static char* name = 0;
#endif

static char*
scores_file (void)
{
#if USER_SCORES_FILE
  if (!name)
    name = strcat_alloc (userdir,"/" SCORES_FILE);
  return name;
#else
  return scoresdir SCORES_FILE;
#endif
}

static int
cmp_scores (const void *r1, const void *r2)
{				/*top_score * r1=c1;
				   top_score * r2=c2; */
  return (((top_score *) r2)->points - ((top_score *) r1)->points);
}

void
sort_scores (void)
{
  int i;
  for (i = 0; i < 5; i++)
    qsort (highs[i], 10, sizeof (top_score), cmp_scores);
}

void
clear_scores (void)
{
  int i, j;
  for (i = 0; i < 5; i++)
    for (j = 0; j < 10; j++) {
      strncpy (highs[i][j].name, "\0", 8);
      highs[i][j].magic = 0;
      highs[i][j].unused1 = 0;
      highs[i][j].unused2 = 0;
      highs[i][j].points = (10 - j) * 250;
    }
}

static unsigned long int
check_scores (void)
{
  unsigned long int crc = 0xa8c4d602;
  unsigned int i;
  char *src = (char *) highs;
  for (i = 0; i < (5 * 10 * sizeof (top_score) - 4); i += 3, src += 3)
    crc ^= *(int *) src;
  return (crc);
}

static void
bswap_scores (void)
{
  int j;

  for (j = 0; j < 50; ++j)
    highs[0][j] = BSWAP32 (highs[0][j]);
}

void
write_scores (void)
{
  FILE *fs;
  unsigned int i;

  fs = fopen (scores_file (), "wb");
  i = check_scores ();
  
  /* convert from local endianess to little-endian */
  bswap_scores ();
  i = BSWAP32 (i);

  /* write scores down to disk */
  fwrite (highs, sizeof (top_score), 50, fs);
  fwrite ((int *) &i, 4, 1, fs);
  fclose (fs);
  
  /* revert scores endianess */
  bswap_scores ();
}

void
load_scores (void)
{
  FILE *fs;
  unsigned long int i;
  fs = fopen (scores_file (), "rb");
  if (fs == NULL)
    clear_scores ();
  else {
    /* read the score from disk */
    fread (highs, sizeof (top_score), 50, fs);
    fread ((int *) &i, 4, 1, fs);
    fclose (fs);

    /* convert from little-endian to local endianess */
    bswap_scores ();
    i = BSWAP32 (i);

    /* check score CRC */
    if (check_scores () != i)
      clear_scores ();
  }
}
