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

#include "common.h"
#include "timer.h"
#include "display.h"
#include "debugmsg.h"

time_type current_time;

void
reset_htimer (htimer_t timer)
{
  timer->orig_time = current_time;
}

void
reset_htimer_with_offset (htimer_t timer, long sec)
{
  reset_htimer (timer);

#ifdef HAVE_GETTIMEOFDAY
  timer->orig_time.tv_sec -= sec;
#else
  timer->orig_time -= sec * SECOND;
#endif
}

htimer_t
new_htimer (enum htimer_kind kind, long slice_duration)
{
  htimer_t result;
  result = malloc (sizeof (htimer_s));
  result->kind = kind;
  result->slice_duration = slice_duration;
  reset_htimer (result);
  dmsg (D_TIMER, "created new timer (kind=%d, slice_duration=%d)=%p",
	kind, slice_duration, &result);
  return result;
}

void
free_htimer (htimer_t timer)
{
  dmsg (D_TIMER, "free timer %p", timer);
  free (timer);
}

void
update_htimers (void)
{
#ifdef HAVE_GETTIMEOFDAY
  gettimeofday (&current_time, 0);
#else
  current_time = get_current_time ();
#endif
}

void
init_htimer (void)
{
  update_htimers ();
}

long
read_htimer (htimer_t timer)
{
#if HAVE_GETTIMEOFDAY
  long s, u, d, res;

  s = current_time.tv_sec - timer->orig_time.tv_sec;
  u = current_time.tv_usec - timer->orig_time.tv_usec;
  d = timer->slice_duration;

  for (;;) {
    /* The following formula computes `(s*SECOND+u)/d', trying to not
       overflow (obviously `s*SECOND+u' is likely to be too big) */
    res = (s*(SECOND/d)) + ((s*(SECOND%d))/d) + ((((s*(SECOND%d))%d)+u)/d);

    if ((res != 0) || (((timer->kind & T_BLOCKING) == 0)))
      break;
    else {			/* The timer is blocking */
      struct timeval present_time;
      gettimeofday (&present_time, 0);
      s = present_time.tv_sec - timer->orig_time.tv_sec;
      u = present_time.tv_usec - timer->orig_time.tv_usec;
    }
  }

  if (timer->kind & T_LOCAL) {
    reset_htimer (timer);
    /* account for the time remaining from the last unfinished slice */
    timer->orig_time.tv_usec -= (((s*(SECOND%d))%d)+u)%d;
    while (timer->orig_time.tv_usec < 0) {
      timer->orig_time.tv_usec += SECOND;
      --timer->orig_time.tv_sec;
    }
  }
#else
  unsigned long c, d, res;

  c = current_time - timer->orig_time;
  d = timer->slice_duration;

  for (;;) {
    res = c / d;

    if ((res != 0) || (((timer->kind & T_BLOCKING) == 0)))
      break;
    else			/* The timer is blocking */
      c = get_current_time () - timer->orig_time;
  }

  if (timer->kind & T_LOCAL) {
    reset_htimer (timer);
    /* account for the time remaining from the last unfinished slice */
    timer->orig_time -= c % d;
  }
#endif
  
  dmsg (D_TIMER, "read timer %p, return %ld", timer, res);
  return res;
}

void
shift_htimer (htimer_t to_shift, htimer_t amount)
{
#if HAVE_GETTIMEOFDAY
  long u,s;

  /* compute the amount to shift the timer with */
  s = current_time.tv_sec - amount->orig_time.tv_sec;
  u = current_time.tv_usec - amount->orig_time.tv_usec;
  if (u < 0) {
    u += SECOND;
    --s;
  }
  
  /* add this amount from the timer's origin */
  to_shift->orig_time.tv_usec += u;
  to_shift->orig_time.tv_sec += s;
  if (to_shift->orig_time.tv_usec >= SECOND) {
    to_shift->orig_time.tv_usec -= SECOND;
    ++to_shift->orig_time.tv_sec;
  }
#else
  to_shift->orig_time += current_time - amount->orig_time;
#endif
  dmsg (D_TIMER, "timer %p shifted", to_shift);
}
