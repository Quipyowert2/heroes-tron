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
#include "savegame.h"
#include "argv.h"
#include "misc.h"
#include "userdir.h"
#include "debugmsg.h"
#include "rsc_files.h"
#include "fopenlock.h"
#include "getshline.h"
#include "errors.h"

#define N_MAGICS 40
saved_game saverec[10];
unsigned char magics[N_MAGICS];

static char *name = 0;
static FILE *fsave = 0;

static char*
saved_games_file (void)
{
  if (!name)
    name = get_non_null_rsc_file ("saved-games-file");
  return name;
}

static void
set_magic (unsigned char i)
{
  assert (i < N_MAGICS);
  magics[i] = 1;
}

/* FIXME: The whole magic system need to be rethought.  */
unsigned char
compute_magic (void)
{
  int i;
  memset (magics, 0, N_MAGICS * sizeof (unsigned char));
  load_scores ();
  for (i = 0; i < 10; i++)
    if (saverec[i].used)
      set_magic (saverec[i].magic);
  for (i = 0; i < 10; i++)
    set_magic (highs[0][i].magic);
  for (i = 1; magics[i] == 1; i++);
  assert (i > 0);
  assert (i < N_MAGICS /* no more magics ?? */ );
  assert (magics[i] == 0);
  return (i);
}

signed char
find_magic (unsigned char m)
{
  int i;
  for (i = 0; i < 10; i++)
    if (highs[0][i].magic == m)
      return (i);
  return (-1);
}

void
clear_save_records (void)
{
  memset (saverec, 0, 10 * sizeof (saved_game));

  if (x10sav) {
    unsigned int i;
    for (i = 0; i < 10; i++) {
      sprintf (saverec[i].name, "LVL%u", (1 + i) * 10);
      saverec[i].level = (1 + i) * 10 - 1;
      saverec[i].points[0] = saverec[i].points[1] = saverec[i].points[2] =
	saverec[i].points[3] = (1 + i) * 1000;
      saverec[i].lifes[0] = saverec[i].lifes[1] = saverec[i].lifes[2] =
	saverec[i].lifes[3] = 9;
      saverec[i].magic = 0;
      saverec[i].used = 1;
    }
  }
}

void
write_save_records (void)
{
  int i;

  if (fsave == 0)
    fsave = fopenlock (saved_games_file (), "wb");

  dmsg (D_FILE, "saving games to %s", saved_games_file ());

  for (i = 0; i < 10; ++i) {
    saved_game *sg = saverec + i;
    fprintf (fsave, "%u %u %u %u %u %u %u %u %u %u %u\n %s\n",
	     sg->level,
	     sg->points[0], sg->lifes[0],
	     sg->points[1], sg->lifes[1],
	     sg->points[2], sg->lifes[2],
	     sg->points[3], sg->lifes[3],
	     sg->magic, sg->used,
	     sg->name);
  }

  fclose (fsave);
  fsave = 0;
}

static void
load_save_records_read (void)
{
  int i;
  int endline = 0;
  char* buf = 0;
  size_t bufsize = 0;
  int firstline = 0;

  clear_save_records ();
  if (fsave == 0)
    return;

  /* Read the scores from disk.  */
  i = 0;
  while (getshline_numbered
         (&firstline, &endline, &buf, &bufsize, fsave) != -1) {
    saved_game *sg = saverec + i;
    if (*buf != ' ') {
      unsigned int u[11];
      if (sscanf (buf, "%u %u %u %u %u %u %u %u %u %u %u",
		  u, u + 1, u + 2, u + 3, u + 4, u + 5, u + 6,
		  u + 7, u + 8, u + 9, u + 10) != 11
	  || u[10] > 1) {
	wmsg (_("%s:%d: parse error.  Clearing saved-game file."),
	      saved_games_file (), firstline);
	clear_scores ();
	return;
      }
      sg->level = u[0];
      sg->points[0] = u[1];
      sg->lifes[0] = u[2];
      sg->points[1] = u[3];
      sg->lifes[1] = u[4];
      sg->points[2] = u[5];
      sg->lifes[2] = u[6];
      sg->points[3] = u[7];
      sg->lifes[3] = u[8];
      sg->magic = u[9];
      sg->used = u[10];
    } else {
      strncpy (sg->name, buf + 1, 16);
      chomp (sg->name);
      ++i;
    }
    /* Exit if the savedgame array is full.  */
    if (i >= 10)
      break;
  }
  free (buf);
}

static void
load_save_records_open (const char *mode)
{
  fsave = fopenlock (saved_games_file (), mode);

  dmsg (D_FILE, "reading saved games from %s", saved_games_file ());

  if (fsave == 0) {
    dmsg (D_FILE, "cannot open %s", saved_games_file ());
    dperror ("fopen");
    clear_save_records ();
    return;
  }
}

void
load_save_records (void)
{
  load_save_records_open ("rt");
  load_save_records_read ();
  if (fsave) {
    fclose (fsave);
    fsave = 0;
  }
}

void
load_save_records_and_keep_locked (void)
{
  load_save_records_open ("r+t");
  load_save_records_read ();
  if (fsave == 0)
    load_save_records_open ("w+t");
  fseek (fsave, 0L, SEEK_SET);
}

void
free_save_records (void)
{
  dmsg (D_MISC, "free save records");

  if (name)
    free (name);
  name = 0;
}
