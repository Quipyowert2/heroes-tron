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
#ifdef HAVE_PKG_GGI

#include <stdlib.h>
#include <stdio.h>
#include <ggi/ggi.h>
#include "display.h"
#include <string.h>
#include "misc.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

unsigned char *screen;

ggi_visual_t visu;
static ggi_visual_t render_visu;
static ggi_mode vid_mode;
static char*	display_params = 0;
int full_screen = 0;

void
set_display_params (const char* str)
{
  if (display_params)
    free (display_params);
  display_params = strdup (str);
}

void set_full_screen_mode (void)
{
  full_screen = 1;
}

void
init_video (void)
{

  const ggi_directbuffer *db;

  if (ggiInit ())
    exit (EXIT_FAILURE);

  visu = ggiOpen (display_params);
  if (!visu) {
    fprintf (stderr, "Failed to open visual.\n");
    exit (EXIT_FAILURE);
  }

  render_visu = ggiOpen ("display-memory",NULL);
  if (!render_visu) {
    fprintf (stderr, "Failed to open an internal `display-memory' visual.\n");
    exit (EXIT_FAILURE);
  }

  vid_mode.frames = 1;
  vid_mode.visible.x = 320;
  vid_mode.visible.y = 200;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GT_8BIT;

  if (ggiSetMode (render_visu, &vid_mode) ||
      /* Try to get a 8bit display */
      ((ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, 320, 200, GT_8BIT,
			   &vid_mode) ||
	ggiSetGraphMode (visu, GGI_AUTO, GGI_AUTO, 320, 200, GT_8BIT)) &&
       /* If unavailable, get any depth available */
       (ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, 320, 200, GGI_AUTO,
			   &vid_mode) ||
	ggiSetGraphMode (visu, GGI_AUTO, GGI_AUTO, 320, 200, GGI_AUTO)))) {
    if (!full_screen || 
	/* try to get any recommanded video mode */
	(ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, 320, 200, GT_8BIT,
			    &vid_mode) &&
	 ggiSetMode (visu, &vid_mode))) {
      fprintf (stderr, "Couldn't setup a correct display.\n");
      exit (EXIT_FAILURE);
    }
  }


  db = ggiDBGetBuffer (render_visu, 0);
  if (!db || !(db->type & GGI_DB_SIMPLE_PLB)) {
    //    printf ("%p\n", db);
    fprintf (stderr, "Can't get correct direct-buffer.\n");
    exit (EXIT_FAILURE);
  }
  screen = db->write;

  ggiAddFlags (visu, GGIFLAG_ASYNC);
  ggiSetColorfulPalette (visu);
  ggiSetColorfulPalette (render_visu);
}

void
uninit_video (void)
{
  ggiClose (render_visu);  
  ggiClose (visu);
  ggiExit ();
}


void
set_color (unsigned char c, unsigned char r, unsigned char g, unsigned char b)
{
  ggi_color cmap[256];
  cmap[c].r = r * 1024;
  cmap[c].g = g * 1024;
  cmap[c].b = b * 1024;
  ggiSetPalette (visu, c, 1, cmap);
  ggiSetPalette (render_visu, c, 1, cmap);
}

void
set_pal (unsigned char *ptr, int p, int n)
{
  signed i;

  ggi_color cmap[256];
  for (i = 0; i < 256; ++i) {
    cmap[i].r = *ptr++ * 1024;
    cmap[i].g = *ptr++ * 1024;
    cmap[i].b = *ptr++ * 1024;
  }
  ggiSetPalette (visu, p / 3, n / 3, cmap);
  ggiSetPalette (render_visu, p / 3, n / 3, cmap);
}

void
vsynchro (void)
{
  ggiCrossBlit (render_visu, 0, 0, 320, 200, visu, 
		(vid_mode.visible.x - 320)/2, 
		(vid_mode.visible.y - 200)/2);
  ggiFlush (visu);
}

#endif
#ifdef HAVE_SDL

#include <stdlib.h>
#include <SDL.h>
#include "display.h"

SDL_Surface* visu;
unsigned char* screen;
int visu_options = SDL_HWPALETTE | SDL_DOUBLEBUF;

void set_display_params (const char* str)
{
  char* s = strcat_alloc ("SDL_VIDEODRIVER=", str);
  putenv (s);
  free (s);
}

void set_full_screen_mode (void)
{
  visu_options |= SDL_FULLSCREEN;
}

/* init the SDL library, this can be called from joystick.c 
   or from init_video() */
void  init_SDL (void);
void 
init_SDL (void)
{
  static int done = 0;

  if (done)
    return;
  SDL_Init (SDL_INIT_VIDEO 
#ifdef HAVE_SDL_JOYSTICKOPEN
	    | SDL_INIT_JOYSTICK
#endif
#ifdef DEBUG
	    | SDL_INIT_NOPARACHUTE
#endif
	    );
  done = 1;
}

void
init_video (void)
{
  init_SDL ();
  visu = SDL_SetVideoMode (320, 200, 8, visu_options);
  if (!visu) {
    fprintf (stderr, "Failed to open visual: %s\n", SDL_GetError());
    exit (EXIT_FAILURE);
  }
  screen = visu->pixels;

  SDL_ShowCursor (0);
  SDL_WM_SetCaption ("Heroes " VERSION,"Heroes");

  /* setup event processing rules.
     FIXME: this do not realy belong to display.c
     it should rather goes to keyb.c */
  SDL_EventState (SDL_ALLEVENTS, SDL_IGNORE);
  SDL_EventState (SDL_KEYDOWN, SDL_ENABLE);
  SDL_EventState (SDL_KEYUP, SDL_ENABLE);
#ifdef HAVE_SDL_ENABLEKEYREPEAT
  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#endif
}

void
uninit_video (void)
{
  SDL_Quit ();  
}

void
set_color (unsigned char c, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_Color cmap[256];
  cmap[c].r = r * 4;
  cmap[c].g = g * 4;
  cmap[c].b = b * 4;
  SDL_SetColors (visu, cmap, c, 1); 
}

void
set_pal (unsigned char *ptr, int p, int n)
{
  signed i;

  SDL_Color cmap[256];
  for (i = 0; i < 256; ++i) {
    cmap[i].r = *ptr++ * 4;
    cmap[i].g = *ptr++ * 4;
    cmap[i].b = *ptr++ * 4;
  }
  SDL_SetColors (visu, cmap, p/3, n/3);
}

void
vsynchro (void)
{
  SDL_Flip (visu);
}

#endif
