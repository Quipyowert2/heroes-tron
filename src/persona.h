/*------------------------------------------------------------------.
| Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
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

#ifndef HEROES_PERSONA_H
#define HEROES_PERSONA_H

/*
 *   This handle the different persona of the process.  The system
 * persona is used to write files in the $(sys-dir) directory.  The
 * user persona is used for every other access.  This makes a
 * difference only if the program has a sgid or suid bit.
 */

/* Get information about the current persona,
   and switch to the user persona.  */
void init_persona (void);
/* Switch to the user persona.  */
void user_persona (void);
/* Switch to the system persona.  */
void sys_persona (void);
/* Open the file whose resource is given, and switch to the system
   persona if the file is located in sys-dir.  */
FILE *
persona_fopenlock (const char *rsc, const char *mode);

#endif /* HEROES_PERSONA_H */
