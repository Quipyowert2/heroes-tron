/*------------------------------------------------------------------------.
| Copyright 2000, 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
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

#ifndef HEROES__ARGV__H
#define HEROES__ARGV__H

/* If the option are beeing read from a file, set from_file to
   the filename so the error message can show it.
   Otherwise set from_file to 0.*/
int parse_argv (int argc, char **argv, const char *from_file, int from_line);

extern int cpuon;
extern bool nosfx;
extern bool joyoff;
extern int devparm;
extern bool loadulevel;
extern bool directmenu;
extern int reinitsco;
extern int reinitopt;
extern int reinitsav;
extern int x10sav;
extern int doublefx;
extern bool swapside;
extern char* level_name;
extern bool mono;
extern bool bits8;
extern bool hqmix;
extern int stretch;
extern bool nosound;
extern bool even_lines;
extern bool showprefs;

#endif /* HEROES__ARGV__H */
