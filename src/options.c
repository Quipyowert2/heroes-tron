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


/* Save/load/re-initialize heroes options. */

#include "system.h"
#include "options.h"
#include "misc.h"
#include "userdir.h"
#include "keys_heroes.h"
#include "bytesex.h"
#include "debugmsg.h"
#include "rsc_files.h"

options_t opt;
char extrasel = 0;

static char* name = 0;

static char*
options_file (void)
{
  if (!name)
    name = get_non_null_rsc_file ("options-file");
  return name;
}

void
reinit_options (void)
{
  opt.screen_size = 0;		/* 0=max */
  opt.radar_map = 1;
  opt.display_infos = 1;
  opt.luminance = 3;
  opt.inertia = 1;
  opt.music = 1;
  opt.music_volume = 6;		/* 0=max */
  opt.sfx = 1;
  opt.sfx_volume = 6;		/* 0=max */
  opt.ctrl_one = 0;		/* 0=keyboard */
  opt.ctrl_two = 0;		/* 1=joystick */
  opt.autopilot_one = 1;
  opt.autopilot_two = 1;
  opt.ghosts = 0;
  opt.speed = 0;
  opt.gamerounds = 4;
  opt.player_color[0] = 0;
  opt.player_color[1] = 1;
  opt.player_color[2] = 2;
  opt.player_color[3] = 3;
  opt.player_keys[0][0] = HK_Up;
  opt.player_keys[0][1] = HK_Left;
  opt.player_keys[0][2] = HK_Down;
  opt.player_keys[0][3] = HK_Right;
  opt.player_keys[0][4] = HK_CtrlR;
  opt.player_keys[0][5] = HK_ShiftR;
  opt.player_keys[1][0] = HK_E;
  opt.player_keys[1][1] = HK_S;
  opt.player_keys[1][2] = HK_D;
  opt.player_keys[1][3] = HK_F;
  opt.player_keys[1][4] = HK_CtrlL;
  opt.player_keys[1][5] = HK_ShiftL;
  opt.extras = 0;
}

/* convert endianess */
static void
bswap_options (void)
{
  opt.gamerounds = BSWAP32 (opt.gamerounds);
  opt.player_color[0] = BSWAP32 (opt.player_color[0]);
  opt.player_color[1] = BSWAP32 (opt.player_color[1]);
  opt.player_color[2] = BSWAP32 (opt.player_color[2]);
  opt.player_color[3] = BSWAP32 (opt.player_color[3]);
  opt.player_keys[0][0] = BSWAP32 (opt.player_keys[0][0]);
  opt.player_keys[0][1] = BSWAP32 (opt.player_keys[0][1]);
  opt.player_keys[0][2] = BSWAP32 (opt.player_keys[0][2]);
  opt.player_keys[0][3] = BSWAP32 (opt.player_keys[0][3]);
  opt.player_keys[0][4] = BSWAP32 (opt.player_keys[0][4]);
  opt.player_keys[0][5] = BSWAP32 (opt.player_keys[0][5]);
  opt.player_keys[1][0] = BSWAP32 (opt.player_keys[1][0]);
  opt.player_keys[1][1] = BSWAP32 (opt.player_keys[1][1]);
  opt.player_keys[1][2] = BSWAP32 (opt.player_keys[1][2]);
  opt.player_keys[1][3] = BSWAP32 (opt.player_keys[1][3]);
  opt.player_keys[1][4] = BSWAP32 (opt.player_keys[1][4]);
  opt.player_keys[1][5] = BSWAP32 (opt.player_keys[1][5]);
}

void
write_options (void)
{
  FILE *fs;
  fs = fopen (options_file (), "wb");

  dmsg (D_FILE, "writing option to %s", options_file ());

  /* convert endianess */
  bswap_options ();

  fwrite (&opt, 1, sizeof (opt), fs);
  fclose (fs);

  /* convert back */
  bswap_options ();
}

void
load_options (void)
{
  FILE *fs;

  dmsg (D_FILE, "reading option from %s", options_file ());
  fs = fopen (options_file (), "rb");

  if (fs == NULL) {
    dmsg (D_FILE, "cannot open %s", options_file ());
    dperror ("fopen");
    reinit_options ();
  }
  else {
    if (fread (&opt, 1, sizeof (opt), fs) != sizeof (opt))
      reinit_options ();
    else
      /* convert endianess */
      bswap_options ();
    fclose (fs);
  }
}

void
free_options (void)
{
  dmsg (D_MISC, "free options");

  if (name)
    free (name);
  name = 0;
}
