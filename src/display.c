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
#include "display.h"
#include "misc.h"
#include "argv.h"
#include "debugmsg.h"
#include "fastmem.h"
#include "errors.h"

pixel_t* screen_rv = 0;		/* A pointer to the screen buffer associated
				   to the render visual. */

/* screen_rv may be a direct pointer to the hardware video buffer, or
   it may be a pointer to system (mallocated) memory, this depends on
   the display driver (if the videomode were available or needed to be
   emulated etc.).  If screen_rv points directly to hardware video
   it might requires locking. */

pixel_t* screen = 0;		/* A pointer to the screen buffer, 
				   used throughout the game
				   (screen is always 320x200). */

char screen_allocated = 0;	/* Whether screen has been mallocated */

/* If no display stretching is needed and screen_rv is not a pointer
   to hardware, then screen == screen_rv.  Otherwise, screen is a
   separate mallocated buffer (screen_allocated==1) whose content is
   stretched or copied to screen_rv before blitting.  This is a kluge
   because when the blit is made accross different depths (common
   case: the game is drawn in 8bits and most display are 16 or 24 bits
   today) the stretching should be performed *during* the crossblit to
   be efficient. */

int scr_w, scr_h;		/* screen_rv width and height */
int scr_pitch;			/* screen_rv pitch */

char video_initialized = 0;	/* has the driver been initialized? */

/* slow stretching routines */

static void
stretch_twofold (void)
{
  pixel_t* s = screen;
  pixel_t* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = 320 / 2; columns_left; --columns_left) {
      pixel_t t1, t2;
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
    d += 2 * scr_pitch - 640;
  }
}

static void
stretch_twofold_even (void)
{
  pixel_t* s = screen;
  pixel_t* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = 320; columns_left; --columns_left) {
      d[1] = d[0] = *s;
      ++s;
      d += 2;
    }
    d += 2 * scr_pitch - 640;
  }
}

static void
stretch_threefold (void)
{
  pixel_t* s = screen;
  pixel_t* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = 320 / 2; columns_left; --columns_left) {
      pixel_t t1, t2;
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
    d += 3 * scr_pitch - 960;
  }
}

static void
stretch_threefold_even (void)
{
  pixel_t* s = screen;
  pixel_t* d = screen_rv;
  int rows_left, columns_left;

  for (rows_left = 200 / 2; rows_left; --rows_left) {
    for (columns_left = 320; columns_left; --columns_left) {
      pixel_t t1, t2;
      t1 = s[0];
      t2 = s[320];
      d[0] = t1;
      d[0+960*4] = t2;
      d[1] = t1;
      d[2] = t1;
      d[1+960*4] = t2;
      d[0+960*2] = t1;
      d[1+960*2] = t1;
      d[2+960*4] = t2;
      d[2+960*2] = t1;
      ++s;
      d += 3;
    }
    d += 6 * scr_pitch - 960;
    s += 320;
  }
}

static void
erase_odd_lines (void)
{
  pixel_t* s = screen+320;
  int i;
  for (i = 100; i; --i, s += 640)
    memset (s, 0, 320);
}

static void
copy_screen (void)
{
  pixel_t* s = screen;
  pixel_t* d = screen_rv;
  int i;
  for (i = 200; i; --i, s += 320, d += scr_pitch)
    fastmem4 (s, d, 320/4);
}

/* Copy the rendered display (screen) to the visual (screen_rv).  This
   may require stretching, if the user asked for.  There may be
   nothing to do (in the case where screen = screen_rv).  */
static void
copy_display (void)
{
  /* the result of stretching routines is written directly
     to the video memory */
  if (stretch == 2) {
    if (even_lines)
      stretch_twofold_even ();
    else
      stretch_twofold ();
  } else if (stretch == 3) {
    if (even_lines)    
      stretch_threefold_even ();
    else
      stretch_threefold ();
  } else {			/* stretch == 1 */
    if (even_lines)    
      erase_odd_lines ();
    if (screen_allocated)
      copy_screen ();
  }
}

#ifdef HAVE_PKG_GGI

ggi_visual_t visu = 0;		/* The real display, which receive events. */
static ggi_visual_t render_visu = 0; /* A 8bit memory-display on which 
					the game is drawn */

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

  dmsg (D_VIDEO|D_SYSTEM, "initialize GGI");
  if (ggiInit ()) {
    dmsg (D_VIDEO, "failed");
    exit (EXIT_FAILURE);
  }

  video_initialized = 1;

  dmsg (D_VIDEO, "open main visual (%s)",
	display_params ? display_params : "null"); 
  visu = ggiOpen (display_params);
  if (!visu)
    emsg ("Failed to open visual.");

  dmsg (D_VIDEO, "open display-memory visual");  
  render_visu = ggiOpen ("display-memory", NULL);
  if (!render_visu)
    emsg ("Failed to open an internal `display-memory' visual.");

  scr_w = 320 * stretch;
  scr_h = 200 * stretch;
  scr_pitch = scr_w;

  vid_mode.frames = 1;
  vid_mode.visible.x = scr_w;
  vid_mode.visible.y = scr_h;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GT_8BIT;

  dmsg (D_VIDEO, "negociate video mode");  
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
      emsg ("Couldn't setup a correct display.");
    }
  }
  dmsg (D_VIDEO, "video mode is %dx%dx%d", 
	vid_mode.visible.x, vid_mode.visible.y, GT_DEPTH (vid_mode.graphtype));
  
  dmsg (D_VIDEO, "ask for a direct-buffer");
  db = ggiDBGetBuffer (render_visu, 0);
  if (!db || !(db->type & GGI_DB_SIMPLE_PLB))
    emsg ("Can't get correct direct-buffer.\n");

  screen_rv = db->write;

  if (stretch > 1) {
    screen = malloc (320*200);
    screen_allocated = 1;
  } else
    screen = screen_rv;

  dmsg (D_VIDEO, "set display flags");
  ggiAddFlags (visu, GGIFLAG_ASYNC);
  if (GT_SCHEME (vid_mode.graphtype) == GT_PALETTE)
    ggiSetColorfulPalette (visu);
  ggiSetColorfulPalette (render_visu);
}

void
uninit_video (void)
{
  if (display_params) {
    dmsg (D_MISC, "free display_params");
    free (display_params);
    display_params = 0;
  }
  if (screen_allocated) {
    dmsg (D_MISC, "free screen buffer");
    free (screen);
    screen = 0;
  }
  if (render_visu) {
    dmsg (D_VIDEO, "close memory visual");
    ggiClose (render_visu);  
    render_visu = 0;
  }
  if (visu) {
    dmsg (D_VIDEO, "close real visual");
    ggiClose (visu);
    visu = 0;
  }
  if (video_initialized) {
    dmsg (D_VIDEO, "exit GGI");
    ggiExit ();
    video_initialized = 0;
  }
}


void
set_color (unsigned char c, unsigned char r, unsigned char g, unsigned char b)
{
  ggi_color cmap[256];
  cmap[c].r = r * 1024;
  cmap[c].g = g * 1024;
  cmap[c].b = b * 1024;
  dmsg (D_VIDEO, "set color %d=(%d,%d,%d)",c,r,g,b);
  ggiSetPalette (visu, c, 1, cmap);
  ggiSetPalette (render_visu, c, 1, cmap);
}

void
set_pal (const unsigned char *ptr, int p, int n)
{
  signed i;

  ggi_color cmap[256];
  for (i = 0; i < 256; ++i) {
    cmap[i].r = *ptr++ * 1024;
    cmap[i].g = *ptr++ * 1024;
    cmap[i].b = *ptr++ * 1024;
  }
  dmsg (D_VIDEO, "set %d colors", n/3);
  ggiSetPalette (visu, p / 3, n / 3, cmap);
  ggiSetPalette (render_visu, p / 3, n / 3, cmap);
}

void
vsynchro (void)
{
  copy_display ();
  ggiCrossBlit (render_visu, 0, 0, scr_w, scr_h, visu, 
		(vid_mode.visible.x - scr_w)/2, 
		(vid_mode.visible.y - scr_h)/2);
  ggiFlush (visu);
}

#endif
#ifdef HAVE_SDL

SDL_Surface* visu = 0;
int visu_options = SDL_HWPALETTE | SDL_DOUBLEBUF;
char SDL_initialized = 0;

void set_display_params (const char* str)
{
  char* s = strcat_alloc ("SDL_VIDEODRIVER=", str);
  dmsg (D_SYSTEM|D_VIDEO,"put `%s' in environment", str);
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
  if (SDL_initialized)
    return;
  dmsg (D_SYSTEM|D_VIDEO|D_JOYSTICK|D_SOUND_TRACK|D_SOUND_EFFECT,
	"initialize SDL");
  SDL_Init (SDL_INIT_VIDEO 
#ifdef HAVE_LIBSDL_MIXER
	    | SDL_INIT_AUDIO
#endif
#ifdef HAVE_SDL_JOYSTICKOPEN
	    | SDL_INIT_JOYSTICK
#endif
#ifdef DEBUG
	    | SDL_INIT_NOPARACHUTE
#endif
	    );
  SDL_initialized = 1;
}

void
init_video (void)
{
  scr_w = 320 * stretch;
  scr_h = 200 * stretch;

  init_SDL ();
  dmsg (D_VIDEO, "set video mode");
  visu = SDL_SetVideoMode (scr_w, scr_h, 8, visu_options);
  /* FIXME: the Linux/m68k binary is crashing in the vicinity */
  if (!visu)
    emsg ("Failed to open visual: %s\n", SDL_GetError());

  video_initialized = 1;

  if (SDL_MUSTLOCK (visu))
    dmsg (D_VIDEO, "visual require locking");
  else
    screen_rv = visu->pixels;

  if (stretch > 1 || SDL_MUSTLOCK (visu)) {
    /* If the game needs stretching or the visual needs locking, we
       don't draw directly on it, we use a separate buffer and
       then copy that buffer to the video memory. */
    screen = malloc (320*200);
    screen_allocated = 1;
  } else
    screen = screen_rv;

  scr_pitch = visu->pitch;

  dmsg (D_VIDEO, "set misc. video parameters");

  SDL_ShowCursor (0);
  SDL_WM_SetCaption ("Heroes " VERSION,"Heroes");

  /* setup event processing rules.
     FIXME: this does not really belong to display.c
     it should rather go to keyb.c */
  SDL_EventState (SDL_ALLEVENTS, SDL_IGNORE);
  SDL_EventState (SDL_KEYDOWN, SDL_ENABLE);
  SDL_EventState (SDL_KEYUP, SDL_ENABLE);
  SDL_EventState (SDL_QUIT, SDL_ENABLE);
#ifdef HAVE_SDL_ENABLEKEYREPEAT
  SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#endif
}

void
uninit_video (void)
{
  if (screen_allocated) {
    dmsg (D_MISC, "free screen buffer");
    free (screen);
    screen = 0;
    screen_allocated = 0;
  }
  if (SDL_initialized) {
    SDL_Quit ();
    SDL_initialized = 0;
  }
}

void
set_color (unsigned char c, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_Color cmap[256];
  cmap[c].r = r * 4;
  cmap[c].g = g * 4;
  cmap[c].b = b * 4;
  dmsg (D_VIDEO, "set color %d=(%d,%d,%d)",c,r,g,b);
  SDL_SetColors (visu, cmap, c, 1); 
}

void
set_pal (const unsigned char *ptr, int p, int n)
{
  signed i;

  SDL_Color cmap[256];
  for (i = 0; i < 256; ++i) {
    cmap[i].r = *ptr++ * 4;
    cmap[i].g = *ptr++ * 4;
    cmap[i].b = *ptr++ * 4;
  }
  dmsg (D_VIDEO, "set %d colors", n/3);
  SDL_SetColors (visu, cmap, p/3, n/3);
}

void
vsynchro (void)
{
  if (SDL_MUSTLOCK (visu))
    SDL_LockSurface (visu);

  screen_rv = visu->pixels;
  copy_display ();

  if (SDL_MUSTLOCK (visu))
    SDL_UnlockSurface (visu);

  SDL_Flip (visu);		/* can change visu->pixels */
}

#endif

