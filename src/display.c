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
#include "misc.h"
#include "argv.h"

unsigned char *screen_rv;	/* A pointer to the screen buffer associated
				   to the render visual. */
unsigned char *screen;		/* A pointer to the screen buffer, 
				   used throughout the game. */

/* When no display stretching is needed screen == screen_rv, otherwise
   screen is a separate mallocated buffer whose content is stretched
   to screen_rv before blitting.  This is a kluge because when the
   blit is made accross different depths (common case: the game is
   drawn in 8bits and most display are 16 or 24 bits today) the
   stretching should be performed *during* the crossblit to be
   efficient. */

static int scr_w, scr_h;	/* visuals width and height */

/* slow stretching routines */

static void
stretch_twofold (void)
{
  unsigned char* s = screen;
  unsigned char* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = 320 / 2; columns_left; --columns_left) {
      unsigned char t1, t2;
      t1 = s[0];
      t2 = s[1];
      d[0] = t1;
      d[640 + 2] = t2;
      d[1] = t1;
      d[640 + 3] = t2;
      d[640 + 0] = t1;
      d[2] = t2;
      d[640 + 1] = t1;
      d[3] = t2;
      s += 2;
      d += 4;
    }
    d += 640;
  }
}

static void
stretch_threefold (void)
{
  unsigned char* s = screen;
  unsigned char* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = 320 / 2; columns_left; --columns_left) {
      unsigned char t1, t2;
      t1 = s[0];
      t2 = s[1];
      d[0] = t1;
      d[960 + 3] = t2;
      d[2*960 + 1] = t1;
      d[4] = t2;
      d[960 + 2] = t1;
      d[2*960 + 5] = t2;
      d[960 + 0] = t1;
      d[2*960 + 3] = t2;
      d[1] = t1;
      d[960 + 4] = t2;
      d[2*960 + 2] = t1;
      d[5] = t2;
      d[2*960 + 0] = t1;
      d[3] = t2;
      d[960 + 1] = t1;
      d[2*960 + 4] = t2;
      d[2] = t1;
      d[960 + 5] = t2;
      s += 2;
      d += 6;
    }
    d += 2 * 960;
  }
}

#ifdef HAVE_PKG_GGI

#include <stdlib.h>
#include <stdio.h>
#include <ggi/ggi.h>
#include "display.h"
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

ggi_visual_t visu;		/* The real display, which receive events. */
static ggi_visual_t render_visu; /* A 8bit memory-display on which the game 
				    is drawn */

static ggi_mode vid_mode;
static char*	display_params = 0;
static int	full_screen = 0;

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

  scr_w = 320 * stretch;
  scr_h = 200 * stretch;

  vid_mode.frames = 1;
  vid_mode.visible.x = scr_w;
  vid_mode.visible.y = scr_h;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GT_8BIT;

  if (ggiSetMode (render_visu, &vid_mode) ||
      /* Try to get a 8bit display */
      ((ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, scr_w, scr_h, GT_8BIT,
			   &vid_mode) ||
	ggiSetGraphMode (visu, GGI_AUTO, GGI_AUTO, scr_w, scr_h, GT_8BIT)) &&
       /* If unavailable, get any depth available */
       (ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, scr_w, scr_h, GGI_AUTO,
			   &vid_mode) ||
	ggiSetGraphMode (visu, GGI_AUTO, GGI_AUTO, scr_w, scr_h, GGI_AUTO)))) {
    if (!full_screen || 
	/* try to get any recommanded video mode */
	(ggiCheckGraphMode (visu, GGI_AUTO, GGI_AUTO, scr_w, scr_h, GT_8BIT,
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
  screen_rv = db->write;

  if (stretch > 1)
    screen = malloc (320*200);
  else
    screen = screen_rv;

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
  if (stretch == 2)
    stretch_twofold ();
  if (stretch == 3)
    stretch_threefold ();
    
  ggiCrossBlit (render_visu, 0, 0, scr_w, scr_h, visu, 
		(vid_mode.visible.x - scr_w)/2, 
		(vid_mode.visible.y - scr_h)/2);
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
  scr_w = 320 * stretch;
  scr_h = 200 * stretch;

  init_SDL ();
  visu = SDL_SetVideoMode (scr_w, scr_h, 8, visu_options);
  if (!visu) {
    fprintf (stderr, "Failed to open visual: %s\n", SDL_GetError());
    exit (EXIT_FAILURE);
  }
  screen_rv = visu->pixels;

  if (stretch > 1)
    screen = malloc (320*200);
  else
    screen = screen_rv;

  SDL_ShowCursor (0);
  SDL_WM_SetCaption ("Heroes " VERSION,"Heroes");

  /* setup event processing rules.
     FIXME: this does not really belong to display.c
     it should rather go to keyb.c */
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
  if (stretch == 2)
    stretch_twofold ();
  if (stretch == 3)
    stretch_threefold ();

  SDL_Flip (visu);
}

#endif
