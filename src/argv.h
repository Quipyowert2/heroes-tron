/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
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

int parse_argv (int argc, char **argv);

extern int snap;
extern int cpuon;
extern int nosfx;
extern int joyoff;
extern int devparm;
extern int loadulevel;
extern int directmenu;
extern int reinitsco;
extern int reinitopt;
extern int reinitsav;
extern int x10sav;
extern int doublefx;
extern int swapside;
extern char* level_name;
extern int mono;
extern int bits8;
extern int hqmix;
extern int stretch;
extern int nosound;
extern int even_lines;

#endif /* HEROES__ARGV__H */
