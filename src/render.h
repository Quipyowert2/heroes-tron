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

#ifndef HEROES__RENDER__H
#define HEROES__RENDER__H

extern char tutor;

void draw_level (int p);
void draw_radar_map (int dx, int dy);
void draw_score (int c, int p, unsigned int dest);
void draw_logo_info (int c, int nbr, pixel_t* dest);
void display_buffer_tmp1 (void);
void display_buffer_moving (int x);
void display_two_buffers (void);
void display_two_buffers_moving (int x);
void display_two_buffers_moving_and_clear (int x);

#endif /* HEROES__RENDER__H */
