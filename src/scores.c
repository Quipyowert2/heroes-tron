/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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

#include "system.h"
#include "structs.h"
#include "scores.h"
#include "misc.h"
#include "userdir.h"
#include "bytesex.h"
#include "debugmsg.h"
#include "rsc_files.h"
#include "fopenlock.h"

top_score highs[5][10];

static char *name = 0;
static FILE *fscores = 0;

static char*
scores_file (void)
{
  if (!name)
    name = get_non_null_rsc_file ("scores-file");
  return name;
}

static int
cmp_scores (const void *r1, const void *r2)
{
  return (((const top_score *) r2)->points - ((const top_score *) r1)->points);
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
  unsigned char *src = (unsigned char *) highs;
  for (i = 0; i < (5 * 10 * sizeof (top_score) - 4); i += 3, src += 3)
    crc ^= GETWORD(src);
  return (crc);
}

static void
bswap_scores (void)
{
  int j;

  for (j = 0; j < 50; ++j)
    highs[0][j].points = BSWAP32 (highs[0][j].points);
}

void
write_scores (void)
{
  unsigned int i;

  if (fscores == 0)
    fscores = fopenlock (scores_file (), "wb");

  dmsg (D_FILE, "writing scores to %s", scores_file ());

  i = check_scores ();

  /* convert from local endianess to little-endian */
  bswap_scores ();
  i = BSWAP32 (i);

  /* write scores down to disk */
  fwrite (highs, sizeof (top_score), 50, fscores);
  fwrite ((int *) &i, 4, 1, fscores);
  fclose (fscores);
  fscores = 0;

  /* revert scores endianess */
  bswap_scores ();
}

static void
load_scores_open (const char *mode)
{
  fscores = fopenlock (scores_file (), mode);
  dmsg (D_FILE, "reading scores from %s", scores_file ());

  if (fscores == 0) {
    dmsg (D_FILE, "cannot open %s", scores_file ());
    dperror ("fopen");
    clear_scores ();
  }
}

static void
load_scores_read (void)
{
  unsigned long int i;

  if (fscores == 0) {
    clear_scores ();
    return;
  }

  /* Read the score from disk.  */
  fread (highs, sizeof (top_score), 50, fscores);
  fread ((int *) &i, 4, 1, fscores);

  /* Convert from little-endian to local endianess.  */
  bswap_scores ();
  i = BSWAP32 (i);

  /* Check score CRC.  */
  if (check_scores () != i)
    clear_scores ();
}

void
load_scores (void)
{
  load_scores_open ("rb");
  load_scores_read ();
  fclose (fscores);
  fscores = 0;
}

void
load_scores_and_keep_locked (void)
{
  load_scores_open ("r+b");
  load_scores_read ();
  fseek (fscores, 0L, SEEK_SET);
}

void
free_scores (void)
{
  dmsg (D_MISC, "free scores");

  if (name)
    free (name);
  name = 0;
}
