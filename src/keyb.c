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
#include "keyb.h"

int enable_mouse = 0;
int mouse_pos_x = 0;
int mouse_pos_y = 0;
char mouse_button_left = 0;
char mouse_button_right = 0;

unsigned char keyboard_map[KEY_MAX + 1];

void
init_keyboard_map (void)
{
  int i;

  for (i = KEY_MAX; i >= 0; i--)
    keyboard_map[i] = 0;
}

int 
mouse_x (void)
{
  return mouse_pos_x;
}

int 
mouse_y (void)
{
  return mouse_pos_y;
}

char 
mouse1 (void)
{
  return mouse_button_left;
}

char 
mouse2 (void)
{
  return mouse_button_right;
}

char 
mouse12 (void)
{
  return mouse_button_left && mouse_button_right;
}

#ifdef HAVE_PKG_GGI

#include <stdio.h>
#include <stdlib.h>
#include <ggi/ggi.h>
#include <assert.h>

extern ggi_visual_t visu;

int
init_mouse (void)
{
  return 0;
}

void
mouse_show (void)
{
  enable_mouse = 1;
}

void
mouse_hide (void)
{
  enable_mouse = 0;
}

void
process_input_events (void)
{
  struct timeval t = { 0, 0 };

  unsigned int mask = emKeyPress | emKeyRelease | emKeyRepeat;

  if (enable_mouse)
    mask |= emPointer;

  if (ggiEventPoll (visu, mask, &t) 
      != emZero) {
    int nbr;
    ggi_event ev;

    nbr = ggiEventsQueued (visu, mask);
    for (; nbr; --nbr) {
      ggiEventRead (visu, &ev, mask);
      switch (ev.any.type) {

	/* keboard events */

      case evKeyPress:
	assert (ev.key.label <= KEY_MAX);
	keyboard_map[ev.key.label] = 1;
	break;
      case evKeyRelease:
	assert (ev.key.label <= KEY_MAX);
	keyboard_map[ev.key.label] = 0;
	break;
      case evKeyRepeat:
	/* NOP */
	break;

	/* mouse events */

      case evPtrAbsolute:
	mouse_pos_x = ev.pmove.x;
	mouse_pos_y = ev.pmove.y;
	break;
      case evPtrRelative:
	mouse_pos_x += ev.pmove.x;
	mouse_pos_y += ev.pmove.y;
	break;
      case evPtrButtonPress:
	if (ev.pbutton.button == GII_PBUTTON_LEFT)
	  mouse_button_left = 1;
	else if (ev.pbutton.button == GII_PBUTTON_RIGHT)
	  mouse_button_right = 1;	  
	break;
      case evPtrButtonRelease:
	if (ev.pbutton.button == GII_PBUTTON_LEFT)
	  mouse_button_left = 0;
	else if (ev.pbutton.button == GII_PBUTTON_RIGHT)
	  mouse_button_right = 0;	  
	break;

      default:
	printf ("unexpected event %d\n", ev.any.type);
      }
    }
  }
}

void
uninit_keyboard_map (void)
{
}

int
get_key (void)
{
  return ggiGetc (visu);
}

int
key_ready (void)
{
  return ggiKbhit (visu);
}

#endif 
#ifdef HAVE_SDL

#include <assert.h>
#include <SDL.h>

int
init_mouse (void)
{
  return 0;
}

void
mouse_show (void)
{
  enable_mouse = 1;
}

void
mouse_hide (void)
{
  enable_mouse = 0;
}

void
process_input_events (void)
{
  SDL_Event ev;
  
  while (SDL_PollEvent (&ev)) {
    if (ev.type == SDL_KEYDOWN) {
      assert (ev.key.keysym.sym <= KEY_MAX);
      keyboard_map[ev.key.keysym.sym] = 1;
    } else if (ev.type == SDL_KEYUP) {
      assert (ev.key.keysym.sym <= KEY_MAX);
      keyboard_map[ev.key.keysym.sym] = 0;
    } else {
      /*      printf ("unexpected event %d\n", ev.type); */
    }
  }
}

void
uninit_keyboard_map (void)
{
}

int
get_key (void)
{
  SDL_Event e;
  int first_time = 1;
  do {
    if (!first_time) {
      SDL_Delay (10);
    } else {
      first_time = 0;
    }
    SDL_PumpEvents ();
    while (SDL_PeepEvents (&e, 1, SDL_GETEVENT, ~SDL_KEYDOWNMASK)) 
      /* NOP */;
  } while (!SDL_PeepEvents (&e, 1, SDL_GETEVENT, SDL_KEYDOWNMASK));
  
  return e.key.keysym.sym;
}

int
key_ready (void)
{
  SDL_PumpEvents();
  return SDL_PeepEvents(0, 1, SDL_GETEVENT, SDL_KEYDOWNMASK);
}

#endif
