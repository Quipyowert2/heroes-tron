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


#ifndef HEROES__DISPLAY__H
#define HEROES__DISPLAY__H

typedef u8_t pixel_t;
extern pixel_t* screen;

void set_display_params (const char* str);
void set_full_screen_mode (void);

void init_video (void);
void uninit_video (void);
void set_color (unsigned char c, unsigned char r, unsigned char g,
		unsigned char b);
void vsynchro (void);
void set_pal (const unsigned char *ptr, int p, int n);

#endif /* HEROES__DISPLAY__H */
