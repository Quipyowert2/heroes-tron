/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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


#ifndef __VISUALS_H_plx__
#define __VISUALS_H_plx__

extern int rotosinus[256];
extern int rotocosinus[256];
extern int rotosinus2[256];
extern int rotocosinus2[256];
extern signed char moyensinus[512];
//extern  char minideform[256];
extern int mulxbuf[200];
extern int angle;
extern char *srcroto;

void rotozoom_buffer (void);	/* rotozoom */
void rotozoom_half_buffer (int c);
void wave_buffer (void);	/* mushroom */
void wave_half_buffer (int c);
void flip_buffer (int p2);	/* renversement */
void corner_buffer (int t2);	/* corner_buffer de page */

void compute_lut (void);	/* init tables sin/cos */

#endif
