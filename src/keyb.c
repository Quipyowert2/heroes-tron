/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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
#include "keyb.h"
#include "argv.h"
#include "errors.h"

int enable_mouse = 0;
int mouse_pos_x = 0;
int mouse_pos_y = 0;
char mouse_button_left = 0;
char mouse_button_right = 0;

unsigned int keyboard_modifiers;
unsigned char keyboard_map[KEY_MAX + 1];

void update_mouse_state (void);

void
init_keyboard_map (void)
{
  int i;

  for (i = KEY_MAX; i >= 0; i--)
    keyboard_map[i] = 0;
  keyboard_modifiers = 0;
}

int 
mouse_x (void)
{
  update_mouse_state ();
  return mouse_pos_x / stretch;
}

int 
mouse_y (void)
{
  update_mouse_state ();
  return mouse_pos_y / stretch;
}

char 
mouse1 (void)
{
  update_mouse_state ();
  return mouse_button_left;
}

char 
mouse2 (void)
{
  update_mouse_state ();
  return mouse_button_right;
}

char 
mouse12 (void)
{
  update_mouse_state ();
  return mouse_button_left || mouse_button_right;
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
	keyboard_modifiers = ev.key.modifiers;
	break;
      case evKeyRelease:
	assert (ev.key.label <= KEY_MAX);
	keyboard_map[ev.key.label] = 0;
	keyboard_modifiers = ev.key.modifiers;
	break;
      case evKeyRepeat:
	/* NOP */
	break;

      default:
	printf ("unexpected event %d\n", ev.any.type);
      }
    }
  }
}

void
update_mouse_state (void)
{
  struct timeval t = { 0, 0 };

  unsigned int mask = emPointer;

  /*
  if (!enable_mouse)
    return;
  */

  if (ggiEventPoll (visu, mask, &t) 
      != emZero) {
    int nbr;
    ggi_event ev;

    nbr = ggiEventsQueued (visu, mask);
    for (; nbr; --nbr) {
      ggiEventRead (visu, &ev, mask);
      switch (ev.any.type) {

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

keycode_t
get_key (void)
{
  ggi_event ev;

  /* Block until we get a key. */
  ggiEventRead(visu, &ev, emKeyPress | emKeyRepeat);

  keyboard_modifiers = ev.key.modifiers;
  return ev.key.sym;
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

/* These macros are defined in SDL 1.1.x but not in 1.0.x */
#ifndef SDL_BUTTON_LEFT
#define SDL_BUTTON_LEFT         1
#endif
#ifndef SDL_BUTTON_MIDDLE
#define SDL_BUTTON_MIDDLE       2
#endif
#ifndef SDL_BUTTON_RIGHT
#define SDL_BUTTON_RIGHT        3
#endif

int
init_mouse (void)
{
  return 0;
}

void
mouse_show (void)
{
  enable_mouse = 1;
  SDL_EventState (SDL_MOUSEMOTION, SDL_ENABLE);
  SDL_EventState (SDL_MOUSEBUTTONUP, SDL_ENABLE);
  SDL_EventState (SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
  SDL_ShowCursor (1);  
}

void
mouse_hide (void)
{
  SDL_ShowCursor (0);
  SDL_EventState (SDL_MOUSEMOTION, SDL_IGNORE);
  SDL_EventState (SDL_MOUSEBUTTONUP, SDL_IGNORE);
  SDL_EventState (SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
  enable_mouse = 0;
}

static int 
handle_mouse_events (const SDL_Event *ev)
{
  if (ev->type == SDL_MOUSEMOTION) {
    mouse_pos_x = ev->motion.x;
    mouse_pos_y = ev->motion.y;
    return 1;
  } else if (ev->type == SDL_MOUSEBUTTONUP) {
    if (ev->button.button == SDL_BUTTON_LEFT)
      mouse_button_left = 0;
    else if (ev->button.button == SDL_BUTTON_RIGHT)
      mouse_button_right = 0;
    return 1;
  } else if (ev->type == SDL_MOUSEBUTTONDOWN) {
    if (ev->button.button == SDL_BUTTON_LEFT)
      mouse_button_left = 1;
    else if (ev->button.button == SDL_BUTTON_RIGHT)
      mouse_button_right = 1;
    return 1;
  }
  return 0;
}

void
update_mouse_state (void)
{
  SDL_Event ev;

  SDL_PumpEvents ();
  while (SDL_PeepEvents (&ev, 1, SDL_GETEVENT, 
			 SDL_MOUSEMOTIONMASK|
			 SDL_MOUSEBUTTONDOWNMASK|
			 SDL_MOUSEBUTTONUPMASK)) {
    handle_mouse_events (&ev);
  }
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
    } else if (handle_mouse_events (&ev)) {
      /* Nothing to do, handle_mouse_events already did everything. */
    } else if (ev.type == SDL_QUIT) {
      exit_heroes (0);
    } else {
      /* printf ("unexpected event %d\n", ev.type); */
    }
  }
  keyboard_modifiers = SDL_GetModState ();
}

void
uninit_keyboard_map (void)
{
}

keycode_t
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
    /* remove all events until we get a KEYDOWN event */
    while (SDL_PeepEvents (&e, 1, SDL_GETEVENT, ~SDL_KEYDOWNMASK)) {
      /* we might need to handle mouse and quit events */
      if (enable_mouse)
	handle_mouse_events (&e);
      if (e.type == SDL_QUIT)
	exit_heroes (0);
    }
  } while (!SDL_PeepEvents (&e, 1, SDL_GETEVENT, SDL_KEYDOWNMASK));
  
  keyboard_modifiers = SDL_GetModState ();
  return e.key.keysym.sym;
}

int
key_ready (void)
{
  SDL_PumpEvents ();
  return SDL_PeepEvents (0, 1, SDL_GETEVENT, SDL_KEYDOWNMASK|SDL_QUITMASK);
  /* return true if there is a pending SDL_QUIT event: the next call to 
     get_key will process it */
}

#endif
