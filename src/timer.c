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


#include "config.h"

#if HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif

#include "timer.h"
#include "display.h"

/*
 * Under DOS, there was a counter upgraded 70 times per second
 * (320x200 is 70Hz) used determinate when to draw the screen, 
 * and alse used as a time unit for the game.  Speeds, animations
 * and other speeds are givent as 'per ticks' values.
 *
 * Here we have to fake such a timer.
 */

unsigned int frame_timer;
struct timeval start_time;
static long usec_frame_lenght;
static long frames_per_second;

void
init_timer (void)
{
  /* 70 FPS */
  frames_per_second = 70;
  usec_frame_lenght = 1000000 / frames_per_second;

  /* init the timer */
  frame_timer = 0;
  gettimeofday (&start_time, 0);
}

void
uninit_timer (void)
{
}

unsigned int
read_and_reset_timer (void)
{
  unsigned int tmp = update_timer ();
  init_timer ();
  return tmp;
}

unsigned int
read_and_set_timer_with_value (int value)
{
  unsigned int tmp = update_timer ();
  long usec, sec;

  gettimeofday (&start_time, 0);
  usec = start_time.tv_usec - (value * usec_frame_lenght) % 1000000;
  sec = start_time.tv_sec - (value * usec_frame_lenght) / 1000000;
  if (usec < 0) {
    usec += 1000000;
    ++sec;
  }
  start_time.tv_usec = usec;
  start_time.tv_sec = sec;
  update_timer ();
  return tmp;
}

unsigned int
read_and_reset_timer_non_zero (void)
{
  unsigned int tmp = read_and_reset_timer ();
  return (tmp ? tmp : 1);
}

unsigned int
update_timer (void)
{
  struct timeval cur;

  gettimeofday (&cur, 0);
  frame_timer = ((cur.tv_sec - start_time.tv_sec) * frames_per_second +
		 (cur.tv_usec - start_time.tv_usec) / usec_frame_lenght);
  return frame_timer;
}

/*
 * block until a least one tick elasped
 */
unsigned int
update_timer_block (unsigned int old)
{
  unsigned int cur;
  cur = update_timer ();
  while (cur == old) {
    // Grrr... Linux time slices are about 1/100 sec, which mean
    // we can't sleep less. :-(
    //    usleep (4000);
    // So what ?  Should we waste CPU time like this ?
    cur = update_timer ();
  }
  return cur;
}
