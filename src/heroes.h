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


#ifndef __HEROES_H_plx__
#define __HEROES_H_plx__

#include "timer.h"

extern timer_t clock_timer;
extern timer_t blink_timer;
extern timer_t bonus_anim_timer;
extern timer_t tiles_anim_timer;
extern timer_t corner_timer;
extern timer_t fading_timer;
extern timer_t waving_timer;
extern timer_t background_timer ;
extern timer_t corner_timer ;
extern timer_t demo_trigger_timer;
extern long event_time;

void compute_corner (int p, int n);
void play_demo (void);

#endif
