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


#ifndef __TIMER_H__
#define __TIMER_H__

#include "config.h"

#if HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif

#define SEC 1000000
#define HZ(x)   (SEC/(x))

/* global timer return durations between the reset of the timer and
   the current time, local timer return durations between two
   successive reads.  Blocking timers will wait until they can
   return on non null number of slices on read_timer. */
enum timer_kind { T_GLOBAL = 0, 
		  T_LOCAL = 1,
		  T_BLOCKING = 2};

typedef struct {
  struct timeval orig_time;
  enum timer_kind kind;
  long slice_duration;		/* duration of a slice in microseconds */
} timer_s;

typedef timer_s* timer_t;

timer_t new_timer (enum timer_kind kind, long slice_duration);
void free_timer (timer_t timer);
void reset_timer (timer_t timer);
void reset_timer_with_offset (timer_t timer, long sec, long usec); 
long read_timer (timer_t timer); /* return elapsed time in slices */
void update_timers (void);
void init_timer (void);
void shift_timer (timer_t to_shift, timer_t amount);

#endif
