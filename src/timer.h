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


#ifndef HEROES__TIMER__H
#define HEROES__TIMER__H

/* If gettimeofday() does not exists, we use timeGetTime(), or clock() */
#if HAVE_GETTIMEOFDAY
# define SECOND 1000000
typedef struct timeval time_type;
#else
# if HAVE_WINDOWS_H
#  define SECOND 1000
#  define get_current_time timeGetTime
typedef DWORD time_type;
# else
#  define SECOND CLOCKS_PER_SEC
#  define get_current_time clock
typedef clock_t time_type;
# endif
#endif

#define HZ(x)   (SECOND/(x))

/* A global timer returns durations between the reset of the timer and
   the current time, a local timer returns durations between two
   successive reads.  Blocking timers will wait until they can return
   on non null number of slices on read_htimer. */
enum htimer_kind { T_GLOBAL = 0, 
		  T_LOCAL = 1,
		  T_BLOCKING = 2};

typedef struct {
  time_type orig_time;
  enum htimer_kind kind;
  long slice_duration;		/* duration of a slice in microseconds */
} htimer_s;

typedef htimer_s* htimer_t;

htimer_t new_htimer (enum htimer_kind kind, long slice_duration);
void free_htimer (htimer_t timer);
void reset_htimer (htimer_t timer);
void reset_htimer_with_offset (htimer_t timer, long sec); 
long read_htimer (htimer_t timer); /* return elapsed time in slices */
void update_htimers (void);
void init_htimer (void);
void shift_htimer (htimer_t to_shift, htimer_t amount);

#endif /* HEROES__TIMER__H */
