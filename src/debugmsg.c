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

#include "common.h"
#include "debugmsg.h"
#include "errors.h"

enum debug_lvl debug_level = 0;
const char* progname = 0;

#ifndef dmsg

#if defined VA_START
void 
dmsg (enum debug_lvl dlvl, const char* msg, ...)
#else
void 
dmsg (dlvl, msg, va_alist)
     enum debug_lvl dlvl;
     const char* msg;
     va_dcl;
#endif
{
  if (dlvl & debug_level) {
#ifdef VA_START
    va_list args;
#endif
    fprintf (stderr, "%s: ", progname);
#ifdef VA_START
    VA_START (args, msg);
# if HAVE_VPRINTF
    vfprintf (stderr, msg, args);
# else
    _doprnt (msg, args, stderr);
# endif /* HAVE_VPRINTF */
    va_end (args);
#else
    fprintf (stderr, msg, va_alist);
#endif /* VA_START */
    putc ('\n', stderr);
    fflush (stderr);
  }  
}

void
dperror (const char* s)
{
  if (debug_level) {
    fprintf (stderr, "%s: ", progname);
    fflush (stderr);
    perror (s);
  }
}

#endif /* !dmsg */

void
dmsg_parse_string (const char* opt)
{
  char* buf;

  if (opt) {
#ifndef USE_HEROES_DEBUG
    wmsg ("Ignoring value of HEROES_DEBUG: recompile Heroes with\n"
	  "the --enable-debug configure option if you want that feature.");
#else
    if (opt[0] == '-' || (opt[0] >= '0' && opt[0] <= '9')) {
      /* the option is a number */
      debug_level = atol (opt);
    } else {
      /* the option is a string */
      buf = strdup (opt);
      opt = strtok (buf, " \t:,|&");
      while (opt) {
	int neg = 0;
	int val;

	if (opt[0] == '-' || opt[0] == '!') {
	  neg = 1;
	  ++opt;
	}

	if (!strcasecmp (opt, "all"))
	  val = -1;
	else if (!strcasecmp (opt, "section"))
	  val = D_SECTION;
	else if (!strcasecmp (opt, "system"))
	  val = D_SYSTEM;
	else if (!strcasecmp (opt, "resource"))
	  val = D_RESOURCE;
	else if (!strcasecmp (opt, "file"))
	  val = D_FILE;
	else if (!strcasecmp (opt, "level"))
	  val = D_LEVEL;
	else if (!strcasecmp (opt, "sound_track"))
	  val = D_SOUND_TRACK;
	else if (!strcasecmp (opt, "sound_effect"))
	  val = D_SOUND_EFFECT;
	else if (!strcasecmp (opt, "video"))
	  val = D_VIDEO;
	else if (!strcasecmp (opt, "joystick"))
	  val = D_JOYSTICK;
	else if (!strcasecmp (opt, "timer"))
	  val = D_TIMER;
	else if (!strcasecmp (opt, "misc"))
	  val = D_MISC;
	else if (!strcasecmp (opt, "fader"))
	  val = D_FADER;
	else {
	  wmsg ("Ignoring unknown debugging option `%s'", opt);
	  goto next_opt;
	}
	
	if (neg)
	  debug_level &= ~val;
	else
	  debug_level |= val;
      next_opt:
	opt = strtok (0, " \t:,|&");
      }
      free (buf);
    }
    dmsg (D_MISC, "set debug level to %x", debug_level);
#endif /* HEROES_DEBUG */
  }
}

void
dmsg_init (const char* prgname)
{
  progname = prgname;

  dmsg_parse_string (getenv ("HEROES_DEBUG"));
}
