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
#include <stdio.h>
#include <stdlib.h>
#include "joystick.h"
#include "debugmsg.h"

int joystick_x[2] = { 0, 0 };	// coord. X
int joystick_y[2] = { 0, 0 };	//        Y
char joystick_b[2] = { 0, 0 };	// buttons (on 2bits)
char joystick_detected = 0;

#ifdef JOYSTICK_SUPPORT
#ifdef HAVE_PKG_GII

#include <ggi/gii.h>

gii_input_t* joystick;

static void
_get_joystick_state (void)
{
  struct timeval t = { 0, 0 };

  if (giiEventPoll (joystick, 
		    emValAbsolute | emKeyPress | emKeyRelease, &t) != emZero) {
    int nbr;
    gii_event ev;
    
    nbr = giiEventsQueued (joystick, 
			   emValAbsolute | emKeyPress | emKeyRelease);
    for (; nbr; --nbr) {
      giiEventRead (joystick, &ev, 
		    emValAbsolute | emKeyPress | emKeyRelease);
      if (ev.any.type == evKeyPress) {
/*  	printf("evKeyPress %d\n",ev.key.button); */
	switch (ev.key.button) {
	case 1:
	  joystick_b[0] |= 1;
	  break;
	case 2:
	  joystick_b[0] |= 2;
	  break;
	case 3:
	  joystick_b[1] |= 1;
	  break;
	case 4:
	  joystick_b[1] |= 2;
	  break;	  
	}
      } else if (ev.any.type == evKeyRelease) {
/*  	printf("evKeyRelease %d\n",ev.key.button); */
	switch (ev.key.button) {
	case 1:
	  joystick_b[0] &= ~1;
	  break;
	case 2:
	  joystick_b[0] &= ~2;
	  break;
	case 3:
	  joystick_b[1] &= ~1;
	  break;
	case 4:
	  joystick_b[1] &= ~2;
	  break;	  
	}
      } else if (ev.any.type == evValAbsolute) {
	/*
	unsigned int i;
	printf("evValAbsolute\n");
	for (i = ev.val.first; i < ev.val.first + ev.val.count; ++i) {
	  printf("value[%d] = %d\n",i,ev.val.value[i]);
	}
	*/
	if (0 == ev.val.first)
	  joystick_x[0] = ev.val.value[0];
	if (1 >= ev.val.first && 1 <= ev.val.first + ev.val.count)
	  joystick_y[0] = ev.val.value[1];
	if (2 >= ev.val.first && 2 <= ev.val.first + ev.val.count)
	  joystick_x[0] = ev.val.value[0];
	if (3 >= ev.val.first && 3 <= ev.val.first + ev.val.count)
	  joystick_y[0] = ev.val.value[0];
      } else {
	printf ("unexpected event %d\n", ev.any.type);
      }
    }    
  }
}

char
joyinit (void)
{
  dmsg (D_JOYSTICK, "initialize joystick");
  giiInit ();
  joystick = giiOpen ("linux-joy",NULL);
  if (!joystick) {
    puts ("No joystick found (run with `-J' to suppress this message).");
    joystick_detected = 0;
  } else
    joystick_detected = 1;
  
  return (joystick_detected);
}

/* Update the state of the joysticks */
void
get_joystick_state (void)
{
  if (joystick_detected)
    _get_joystick_state ();
}

#endif /* HAVE_PKG_GII */
#ifdef HAVE_SDL_JOYSTICKOPEN
#include <SDL.h>

SDL_Joystick* joystick[2] = { 0, 0 }; 
int joystick0_butnbr = 0;	/* The number of button of 
				   the FIRST joystick */

extern void init_SDL (void);

char joyinit (void)
{
  int nbr;

  init_SDL ();

  dmsg (D_JOYSTICK, "initialize joystick");

  nbr = SDL_NumJoysticks ();

  if (nbr <= 0) {
    puts ("No joystick found (run with `-J' to suppress this message).");
    return joystick_detected = 0;
  }

  dmsg (D_JOYSTICK, "%d joystick found", nbr);

  joystick[0] = SDL_JoystickOpen (0);
  if (nbr >= 2)
    joystick[1] = SDL_JoystickOpen(1);
  SDL_JoystickEventState (SDL_IGNORE);
  joystick0_butnbr = SDL_JoystickNumButtons (joystick[0]);
  return joystick_detected = (joystick[0] != 0);
}

void get_joystick_state (void)
{
  SDL_JoystickUpdate();
  if (joystick[0]) {
    joystick_b[0] = (joystick_b[0] & ~1) | 
      (SDL_JoystickGetButton (joystick[0], 0) & 1);
    joystick_b[0] = (joystick_b[0] & ~2) | 
      ((SDL_JoystickGetButton (joystick[0], 1) & 1) << 1);
    joystick_x[0] = SDL_JoystickGetAxis(joystick[0], 0);
    joystick_y[0] = SDL_JoystickGetAxis(joystick[0], 1);
    if (!joystick[1] && joystick0_butnbr >= 4) {
      joystick_b[1] = (joystick_b[1] & ~1) | 
	(SDL_JoystickGetButton (joystick[0], 2) & 1);
      joystick_b[1] = (joystick_b[1] & ~2) | 
	((SDL_JoystickGetButton (joystick[0], 3) & 1) << 1);
    }
  }
  if (joystick[1]) {
    joystick_b[1] = (joystick_b[1] & ~1) | 
      (SDL_JoystickGetButton (joystick[1], 0) & 1);
    joystick_b[1] = (joystick_b[1] & ~2) | 
      ((SDL_JoystickGetButton (joystick[1], 1) & 1) << 1);
    joystick_x[1] = SDL_JoystickGetAxis(joystick[1], 0);
    joystick_y[1] = SDL_JoystickGetAxis(joystick[1], 1);
  }
}

#endif /* HAVE_SDL_JOYSTICKOPEN */
#else

char joyinit (void)
{
  return joystick_detected = 0;
}

void get_joystick_state (void)
{
}

#endif
