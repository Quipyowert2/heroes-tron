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

#ifdef HAVE_GETTIMEOFDAY
struct timeval current_time;
#else
clock_t current_time;
#endif

void
reset_timer (timer_t timer)
{
  timer->orig_time = current_time;
}

void
reset_timer_with_offset (timer_t timer, long sec)
{
  reset_timer (timer);

#ifdef HAVE_GETTIMEOFDAY
  timer->orig_time.tv_sec -= sec;
#else
  timer->orig_time -= sec * SEC;
#endif
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
#ifdef GETTIMEOFDAY
  gettimeofday (&current_time, 0);
#else
  current_time = clock ();
#endif
}

void
init_timer (void)
{
  update_timers ();
}

long
read_timer (timer_t timer)
{
#if HAVE_GETTIMEOFDAY
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
      --timer->orig_time.tv_sec;
    }
  }

  if ((res == 0) && (timer->kind & T_BLOCKING)) {
    struct timeval present_time;
    gettimeofday (&present_time, 0);
    s = present_time.tv_sec - timer->orig_time.tv_sec;
    u = present_time.tv_usec - timer->orig_time.tv_usec;
    goto blocking_loop;
  }
#else
  long c, d, res;

  c = current_time - timer->orig_time;
  d = timer->slice_duration;

 blocking_loop:
  res = c / d;

  if (timer->kind & T_LOCAL) {
    reset_timer (timer);
    /* account for the time remaining from the last unfinished slice */
    timer->orig_time -= c % d;
  }

  if ((res == 0) && (timer->kind & T_BLOCKING)) {
    c = clock () - timer->orig_time;
    goto blocking_loop;
  }
#endif
  return res;
}

void
shift_timer (timer_t to_shift, timer_t amount)
{
#if HAVE_GETTIMEOFDAY
  long u,s;

  /* compute the amount to shift the timer with */
  s = current_time.tv_sec - amount->orig_time.tv_sec;
  u = current_time.tv_usec - amount->orig_time.tv_usec;
  if (u < 0) {
    u += SEC;
    --s;
  }
  
  /* add this amount from the timer's origin */
  to_shift->orig_time.tv_usec += u;
  to_shift->orig_time.tv_sec += s;
  if (to_shift->orig_time.tv_usec >= SEC) {
    to_shift->orig_time.tv_usec -= SEC;
    ++to_shift->orig_time.tv_sec;
  }
#else
  clock_t c;

  to_shift->orig_time += current_time - amount->orig_time;
#endif
}
