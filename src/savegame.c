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
#include "savegame.h"
#include "argv.h"
#include "misc.h"
#include "userdir.h"
#include "bytesex.h"
#include "debugmsg.h"
#include "rsc_files.h"

#define N_MAGICS 40
saved_game saverec[10];
unsigned char magics[N_MAGICS];

static char* name = 0;

static char*
saves_file (void)
{
  if (!name)
    name = get_non_null_rsc_file ("games-file");
  return name;
}

static void
set_magic (unsigned char i)
{
  assert (i < N_MAGICS);
  magics[i] = 1;
}

unsigned char
compute_magic (void)
{
  int i;
  memset (magics, 0, N_MAGICS * sizeof (unsigned char));
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


static unsigned long int
check_save_records (void)
{
  unsigned long int crc = 0xa8c4d602;
  unsigned int i;
  unsigned char *src = (unsigned char *) saverec;
  for (i = 0; i < (10 * sizeof (saved_game) - 4); i += 3, src += 3)
    crc ^= GETWORD(src);
  return (crc);
}

void
clear_save_records (void)
{
  memset (saverec, 0, 10 * sizeof (saved_game));
}

/* convert to and from local endianess */
static void
bswap_save_records (saved_game* from, saved_game* to)
{
  int i;

  for (i = 0; i < 10; ++i) {
    strcpy (to[i].name, from[i].name);
    to[i].used = from[i].used;
    to[i].magic = from[i].magic;
    to[i].level = BSWAP32 (from[i].level);
    to[i].points[0] = BSWAP32 (from[i].points[0]);
    to[i].points[1] = BSWAP32 (from[i].points[1]);
    to[i].points[2] = BSWAP32 (from[i].points[2]);
    to[i].points[3] = BSWAP32 (from[i].points[3]);
    to[i].lifes[0] = BSWAP32 (from[i].lifes[0]);
    to[i].lifes[1] = BSWAP32 (from[i].lifes[1]);
    to[i].lifes[2] = BSWAP32 (from[i].lifes[2]);
    to[i].lifes[3] = BSWAP32 (from[i].lifes[3]);
  }
}

void
write_save_records (void)
{
  FILE *fs;
  unsigned int i;
  saved_game savetmp[10];

  fs = fopen (saves_file (), "wb");

  dmsg (D_FILE, "saving games to %s", saves_file ());

  i = check_save_records ();

  /* convert endianess */
  bswap_save_records (saverec, savetmp);
  i = BSWAP32 (i);

  fwrite (savetmp, sizeof (saved_game), 10, fs);
  fwrite ((int *) &i, 4, 1, fs);
  fclose (fs);
}

void
load_save_records (void)
{
  FILE *fs;
  unsigned long int i;
  saved_game savetmp[10];

  fs = fopen (saves_file (), "rb");

  dmsg (D_FILE, "reading saved games from %s", saves_file ());

  if (fs == NULL) {
    dmsg (D_FILE, "cannot open %s", saves_file ());
    dperror ("fopen");
    clear_save_records ();
  } else {
    fread (savetmp, sizeof (saved_game), 10, fs);
    fread ((int *) &i, 4, 1, fs);

    /* convert endianess */
    bswap_save_records (savetmp, saverec);
    i = BSWAP32 (i);

    fclose (fs);
    if (check_save_records () != i)
      clear_save_records ();
  }
  if (x10sav)
    for (i = 0; i < 10; i++) {
      sprintf (saverec[i].name, "LVL%ld", (1 + i) * 10);
      saverec[i].level = (1 + i) * 10 - 1;
      saverec[i].points[0] = saverec[i].points[1] = saverec[i].points[2] =
	saverec[i].points[3] = (1 + i) * 1000;
      saverec[i].lifes[0] = saverec[i].lifes[1] = saverec[i].lifes[2] =
	saverec[i].lifes[3] = 9;
      saverec[i].magic = 0;
      saverec[i].used = 1;
    }
}

void
free_save_records (void)
{
  dmsg (D_MISC, "free save records");

  if (name)
    free (name);
  name = 0;
}
