/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
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
#include "structs.h"
#include "scores.h"
#include "misc.h"
#include "userdir.h"
#include "debugmsg.h"
#include "rsc_files.h"
#include "getshline.h"
#include "errors.h"
#include "persona.h"

top_score highs[5][10];

static char *name = 0;
static FILE *fscores = 0;

static char*
score_file (void)
{
  if (!name)
    name = get_non_null_rsc_file ("score-file");
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

int
find_score_by_gameid (gameid_ptr gid)
{
  int i;
  for (i = 0; i < 10; i++)
    if (equal_gameid (highs[0][i].gid, gid))
      return (i);
  return (-1);
}

void
clear_scores (void)
{
  int i, j;
  for (i = 0; i < 5; i++)
    for (j = 0; j < 10; j++) {
      strncpy (highs[i][j].name, "\0", 8);
      empty_gameid (highs[i][j].gid);
      highs[i][j].points = (10 - j) * 250;
    }
}

void
write_scores (void)
{
  unsigned int i, j;

  if (fscores == 0) {
    fscores = persona_fopenlock ("score-file", "wt");
    if (fscores == 0) {
      wmsg (_("cannot write %s"), score_file ());
      dperror (score_file ());
    }
  } else {
    fflush (fscores);
    if (ftruncate (fileno (fscores), 0) != 0)
      emsg (_("%s: truncate error"), score_file ());
  }

  if (fscores) {
    dmsg (D_FILE, "writing scores to %s", score_file ());

    /* Write down scores to disk.  */
    for (i = 0; i < 5; ++i)
      for (j = 0; j < 10; ++j) {
	char *gidtxt = gameid_to_text (highs[i][j].gid);
	fprintf (fscores, "%u %u %u\n %s\n  %s\n",
		 i, j, highs[i][j].points, gidtxt, highs[i][j].name);
	free (gidtxt);
      }

    fclose (fscores);
    fscores = 0;
  }

  user_persona ();
}

static void
load_scores_open (const char *mode)
{
  dmsg (D_FILE, "reading scores from %s", score_file ());
  fscores = persona_fopenlock ("score-file", mode);

  if (fscores == 0) {
    dmsg (D_FILE, "cannot open %s", score_file ());
    dperror ("fopen");
    clear_scores ();
  }
}

static void
load_scores_error (int line)
{
  wmsg (_("%s:%d: parse error.  Clearing score table."),
	score_file (), line);
  clear_scores ();
}

static void
load_scores_read (void)
{
  unsigned int i, j;
  u32_t points;
  int endline = 0;
  char* buf = 0;
  size_t bufsize = 0;
  int firstline = 0;

  clear_scores ();

  if (fscores == 0)
    return;

  /* Read the scores from disk.  */
  while (getshline_numbered
         (&firstline, &endline, &buf, &bufsize, fscores) != -1) {
    if (*buf != ' ') {
      if (sscanf (buf, "%u %u %u", &i, &j, &points) != 3
	  || i >= 5
	  || j >= 10) {
	load_scores_error (firstline);
	return;
      }
      highs[i][j].points = points;
    } else if (buf[1] != ' ') {
      if (text_to_gameid (buf + 1, highs[i][j].gid)) {
	load_scores_error (firstline);
	return;
      }
    } else {
      strncpy (highs[i][j].name, buf + 2, PLAYER_NAME_SIZE + 1);
      chomp (highs[i][j].name);
    }
  }
  free (buf);
}

void
load_scores (void)
{
  load_scores_open ("rt");
  load_scores_read ();
  if (fscores) {
    fclose (fscores);
    fscores = 0;
  }
}

void
load_scores_and_keep_locked (void)
{
  sys_persona ();
  load_scores_open ("r+t");
  load_scores_read ();
  if (fscores == 0)
    load_scores_open ("w+t");
  if (fscores)
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
