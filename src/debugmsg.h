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

/* Debugging messages facility */

#ifndef HEROES__DEBUGMSG__H
#define HEROES__DEBUGMSG__H

enum debug_lvl { D_SECTION = 1,
		 D_SYSTEM = 4,
		 D_RESOURCE = 8,
		 D_FILE = 16,
		 D_LEVEL = 32,
		 D_SOUND_TRACK = 64,
		 D_SOUND_EFFECT = 128,
		 D_VIDEO = 256,
		 D_JOYSTICK = 512,
		 D_TIMER = 1024,
		 D_MISC = 2048,
		 D_FADER = 4096
};

extern enum debug_lvl debug_level;

#ifndef USE_HEROES_DEBUG
# define dmsg while (0) while
# define dperror while (0) while
#else
# ifdef VA_START
void dmsg (enum debug_lvl dlvl, const char* msg, ...);
# else
void dmsg (void);
# endif
void dperror (const char* s);
#endif

void dmsg_parse_string (const char* opt);
void dmsg_init (const char* prgname);

#endif /* HEROES__DEBUGMSG__H */
