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

#include <stdlib.h>
#include "config.h"
#include "timer.h"
#include "display.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

struct timeval current_time;

void
reset_timer (timer_t timer)
{
  timer->orig_time = current_time;
}

void
reset_timer_with_offset (timer_t timer, long sec, long usec)
{
  reset_timer (timer);

  timer->orig_time.tv_usec -= usec;
  timer->orig_time.tv_sec -= sec;
  if (timer->orig_time.tv_usec < 0) {
    timer->orig_time.tv_usec += SEC;
    ++timer->orig_time.tv_sec;
  }
}

timer_t
new_timer (enum timer_kind kind, long slice_duration)
{
  timer_t result;
  result = malloc (sizeof (timer_s));
  result->kind = kind;
  result->slice_duration = slice_duration;
  reset_timer (result);
  return result;
}

void
free_timer (timer_t timer)
{
  free (timer);
}

void
update_timers (void)
{
  gettimeofday (&current_time, 0);
}

void
init_timer (void)
{
  update_timers ();
}

long
read_timer (timer_t timer)
{
  long s, u, d, res;

  s = current_time.tv_sec - timer->orig_time.tv_sec;
  u = current_time.tv_usec - timer->orig_time.tv_usec;
  d = timer->slice_duration;

 blocking_loop:
  /* The following formula computes `(s*SEC+u)/d', trying to not
     overflow (obviously `s*SEC+u' is likely to be too big) */
  res = (s*(SEC/d)) + ((s*(SEC%d))/d) + ((((s*(SEC%d))%d)+u)/d);

  if (timer->kind & T_LOCAL) {
    reset_timer (timer);
    /* account for the time remaining from the last unfinished slice */
    timer->orig_time.tv_usec -= (((s*(SEC%d))%d)+u)%d;
    while (timer->orig_time.tv_usec < 0) {
      timer->orig_time.tv_usec += SEC;
      ++timer->orig_time.tv_sec;
    }
  }

  if ((res == 0) && (timer->kind & T_BLOCKING)) {
    struct timeval present_time;
    gettimeofday (&present_time, 0);
    s = present_time.tv_sec - timer->orig_time.tv_sec;
    u = present_time.tv_usec - timer->orig_time.tv_usec;
    goto blocking_loop;
  }
  return res;
}

