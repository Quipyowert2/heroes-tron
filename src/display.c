/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License as published by |
| the Free Software Foundation; either version 2 of the License, or |
| (at your option) any later version.                               |
|                                                                   |
| Heroes is distributed in the hope that it will be useful, but     |
| WITHOUT ANY WARRANTY; without even the implied warranty of        |
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU |
| General Public License for more details.                          |
|                                                                   |
| You should have received a copy of the GNU General Public License |
| along with this program; if not, write to the Free Software       |
| Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA          |
| 02111-1307 USA                                                    |
`------------------------------------------------------------------*/

#include "system.h"
#include "display.h"
#include "misc.h"
#include "argv.h"
#include "debugmsg.h"
#include "fastmem.h"
#include "errors.h"

int scr_w, scr_h;		/* rendering buffer width and height */
int scr_pitch;			/* rendering buffer pitch */

char video_initialized = 0;	/* has the driver been initialized? */

/* slow stretching routines */

static void
stretch_twofold (const pixel_t *s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = width / 2; columns_left; --columns_left) {
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
    d += 2 * (scr_pitch - width);
    s += xbuf - width;
  }
}

static void
stretch_twofold_even (const pixel_t *s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = width; columns_left; --columns_left) {
      d[1] = d[0] = *s;
      ++s;
      d += 2;
    }
    d += 2 * (scr_pitch - width);
    s += xbuf - width;
  }
}

static void
stretch_threefold (const pixel_t* s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    for (columns_left = width / 2; columns_left; --columns_left) {
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
    d += 3 * (scr_pitch - width);
    s += xbuf - width;
  }
}

static void
stretch_threefold_even (const pixel_t *s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200 / 2; rows_left; --rows_left) {
    for (columns_left = width; columns_left; --columns_left) {
      pixel_t t1, t2;
      t1 = s[0];
      t2 = s[xbuf];
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
    d += 3 * (2 * scr_pitch - width);
    s += 2 * xbuf - width;
  }
}

static void
stretch_fourfold (const pixel_t *s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    u32_t *d2 = (unsigned int *)d;
    for (columns_left = width; columns_left; --columns_left) {
      pixel_t c = *s;
      u32_t i = (c << 24) | (c << 16) | (c << 8) | c;
      d2[0] = i;
      d2[320] = i;
      d2[320*2] = i;
      d2[320*3] = i;
      ++s;
      ++d2;
    }
    d += 4 * scr_pitch;
    s += xbuf - width;
  }
}

static void
stretch_fourfold_even (const pixel_t* s, pixel_t *d, unsigned width)
{
  int rows_left, columns_left;

  for (rows_left = 200; rows_left; --rows_left) {
    u32_t *d2 = (unsigned int *)d;
    for (columns_left = width; columns_left; --columns_left) {
      pixel_t c = *s;
      u32_t i = (c << 24) | (c << 16) | (c << 8) | c;
      d2[0] = i;
      d2[320*2] = i;
      ++s;
      ++d2;
    }
    d += 4 * scr_pitch;
    s += xbuf - width;
  }
}

static void
copy_screen_even (const pixel_t *s, pixel_t *d, unsigned width)
{
  int i;
  for (i = 200; i; --i, s += xbuf * 2, d += 2 * scr_pitch)
    fastmem4 (s, d, width / 4);
}

static void
copy_screen (const pixel_t *s, pixel_t *d, unsigned width)
{
  int i;
  for (i = 200; i; --i, s += xbuf, d += scr_pitch)
    fastmem4 (s, d, width / 4);
}

/* Copy the rendered display (s) to the visual (screen_rv).  This
   may require stretching, if the user asked for.  */
static void
copy_display (const pixel_t *s, pixel_t *d, unsigned width)
{
  /* the result of stretching routines is written directly
     to the video memory */
  if (stretch == 2) {
    if (even_lines)
      stretch_twofold_even (s, d, width);
    else
      stretch_twofold (s, d, width);
  } else if (stretch == 3) {
    if (even_lines)
      stretch_threefold_even (s, d, width);
    else
      stretch_threefold (s, d, width);
  } else if (stretch == 4) {
    if (even_lines)
      stretch_fourfold_even (s, d, width);
    else
      stretch_fourfold (s, d, width);
  } else {			/* stretch == 1 */
    if (even_lines)
      copy_screen_even (s, d, width);
    else
      copy_screen (s, d, width);
  }
}

#ifdef HAVE_LIBGGI
/* LibGGI driver for Heroes.

   Heroes is a 320x200x8bit game so we try to use such a video mode.
   That's not always possible, so we negociate the closest video mode
   available.

   When the opened video mode is not 8bit depth, another intermediate
   visual (`render_visu') is opened an memory to hold the 8bit
   rendered screens; and this visual is CrossBlited to the main visual
   by LibGGI.

   Hence, there are two different drawing mode: either direct drawing
   to the real visual (`visu') if it has a 8bit depth, or drawing to
   the intermediate visual (`render_visu').  Testing for render_visu
   == NULL, is enough to know in which mode we are.

   We use double buffering for `visu' whenever possible.
*/

ggi_visual_t visu = 0;		/* The real display, which receive events. */
static ggi_visual_t render_visu = NULL; /* A 8bit memory-display on which
					   the game is drawn, if visu is not
					   8bit.*/

static ggi_mode vid_mode;
static char *display_params = NULL;
static int full_screen = 0;
/* Direct buffer for each frame.
   We might have db[0] == db[1] if double buffering is not available.*/
static const ggi_directbuffer *db[2] = { NULL, NULL };
static int current_frame = 0;	/* used as index to `db' for
				   double buffering */

void
set_display_params (const char* str)
{
  XFREE0 (display_params);
  display_params = xstrdup (str);
}

void
set_full_screen_mode (void)
{
  full_screen = 1;
}

static bool
setup_320x200x8_display (void)
{
  dmsg (D_VIDEO, "negociate 320x200x8/2 mode");
  vid_mode.frames = 2;
  vid_mode.visible.x = scr_w;
  vid_mode.visible.y = scr_h;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GT_8BIT;

  if (ggiCheckMode (visu, &vid_mode) == 0)
    return ggiSetMode (visu, &vid_mode) == 0;
  return false;
}

static bool
setup_320x200xB_display (void)
{
  dmsg (D_VIDEO, "negociate any 320x200xB/2 mode");
  vid_mode.frames = 2;
  vid_mode.visible.x = scr_w;
  vid_mode.visible.y = scr_h;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GGI_AUTO;

  if (ggiCheckMode (visu, &vid_mode) == 0)
    return ggiSetMode (visu, &vid_mode) == 0;
  return false;
}

static bool
setup_WWWxHHHxB_display (void)
{
  dmsg (D_VIDEO, "negociate any mode");
  vid_mode.frames = 2;
  vid_mode.visible.x = scr_w;
  vid_mode.visible.y = scr_h;
  vid_mode.virt.x = vid_mode.virt.y = GGI_AUTO;
  vid_mode.dpp.x = vid_mode.dpp.y = GGI_AUTO;
  vid_mode.graphtype = GT_8BIT;

  ggiCheckMode (visu, &vid_mode);
  return ggiSetMode (visu, &vid_mode) == 0;
}

static void
get_directbuffers (ggi_visual_t v)
{
  int numbuf;
  int i;

  dmsg (D_VIDEO, "ask for direct-buffers");
  numbuf = ggiDBGetNumBuffers (v);
  dmsg (D_VIDEO, "%d direct buffers available", numbuf);
  for (i = 0; i < numbuf; ++i) {
    const ggi_directbuffer *d = ggiDBGetBuffer (v, i);
    if (d == NULL)
      continue;
    dmsg (D_VIDEO, "DB#%d: type=%x frame=%d", i, d->type, d->frame);
    if (!(d->type & GGI_DB_SIMPLE_PLB))
      continue;
    if (db[d->frame] == NULL)
      db[d->frame] = d;
  }

  if (db[0] != NULL && db[1] == NULL) {
    dmsg (D_VIDEO, "got only the first direct buffer");
    db[1] = db[0];
  } else if (db[0] == NULL && db[1] != NULL) {
    dmsg (D_VIDEO, "got only the second direct buffer ???");
    db[0] = db[1];
  } else if (db[0] == NULL && db[1] == NULL)
    /* FIXME: ask to send a bug report with a trace */
    emsg (_("failed to get any direct buffer for this visual"));
}

void
init_video (void)
{
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
    /* TRANS: a `visual' is associated by LibGGI to a display.  You can
       actually call it a `display', this makes no big difference.  */
    emsg (_("failed to open visual."));

  scr_w = 320 * stretch;
  scr_h = 200 * stretch;

  if (!(setup_320x200x8_display () ||
	setup_320x200xB_display () ||
	setup_WWWxHHHxB_display ()))
    emsg (_("cannot setup a correct display."));
  /* get info about the mode that was selected */
  ggiGetMode (visu, &vid_mode);
  dmsg (D_VIDEO, "video mode is %dx%dx%d/%d",
	vid_mode.visible.x, vid_mode.visible.y,
	GT_DEPTH (vid_mode.graphtype), vid_mode.frames);
  if (vid_mode.virt.x < 320 || vid_mode.virt.y < 200)
    emsg (_("negociated video mode is too small (width=%d, height=%d)"),
	  vid_mode.virt.x, vid_mode.virt.y);
  if (GT_DEPTH (vid_mode.graphtype) == 8) {
    /* no intermediate buffer required */
    render_visu = NULL;
    scr_pitch = vid_mode.virt.x;
  } else {
    /* open an intermediate rendefing buffer */
    dmsg (D_VIDEO, "open display-memory visual");
    render_visu = ggiOpen ("display-memory", NULL);
    if (render_visu == NULL)
      /* TRANS: `display-memory' is the name of a LibGGI driver and
	 make no sense to translate. */
      emsg (_("Failed to open an internal `display-memory' visual."));
    if (ggiSetGraphMode (render_visu, scr_w, scr_h, scr_w, scr_h, GT_8BIT))
      emsg (_("Failed to set 320x200x8 mode on display-memory"));
    scr_pitch = scr_w;
  }

  get_directbuffers (render_visu != NULL ? render_visu : visu);

  dmsg (D_VIDEO, "set display flags");
  ggiAddFlags (visu, GGIFLAG_ASYNC);
  if (GT_SCHEME (vid_mode.graphtype) == GT_PALETTE)
    ggiSetColorfulPalette (visu);
  if (render_visu != NULL)
    ggiSetColorfulPalette (render_visu);
}

void
uninit_video (void)
{
  dmsg (D_MISC, "uninitialize video");
  XFREE0 (display_params);
  if (render_visu != NULL) {
    dmsg (D_VIDEO, "close memory visual");
    ggiClose (render_visu);
    render_visu = NULL;
  }
  if (visu != NULL) {
    dmsg (D_VIDEO, "close real visual");
    ggiClose (visu);
    visu = NULL;
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
  if (GT_SCHEME (vid_mode.graphtype) == GT_PALETTE)
    ggiSetPalette (visu, c, 1, cmap);
  if (render_visu != NULL)
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
  if (GT_SCHEME (vid_mode.graphtype) == GT_PALETTE)
    ggiSetPalette (visu, p / 3, n / 3, cmap);
  if (render_visu != NULL)
    ggiSetPalette (render_visu, p / 3, n / 3, cmap);
}

void
vsynchro (const pixel_t *s)
{
  vsynchro2 (s, NULL);
}

void
vsynchro2 (const pixel_t *s1, const pixel_t *s2)
{
  pixel_t *dest;

  current_frame ^= 1;

  if (ggiResourceAcquire (db[current_frame]->resource, GGI_ACTYPE_WRITE))
    wmsg (_("failed to acquire direct-buffer"));

  dest = db[current_frame]->write;

  if (render_visu == NULL)
    /* center */
    dest += (vid_mode.visible.x - scr_w)/2 +
      scr_pitch * (vid_mode.visible.y - scr_h)/2;

  if (s2 == NULL) {
    copy_display (s1, dest, 320);
  } else {
    copy_display (s1, dest, 160);
    copy_display (s2, dest + 160 * stretch, 160);
  }

  if (ggiResourceRelease (db[current_frame]->resource))
    wmsg (_("failed to release direct-buffer"));

  if (render_visu != NULL) {
    ggiSetWriteFrame(visu, current_frame);
    ggiCrossBlit (render_visu, 0, 0, scr_w, scr_h, visu,
		  (vid_mode.visible.x - scr_w)/2,
		  (vid_mode.visible.y - scr_h)/2);
  }

  ggiSetDisplayFrame (visu, current_frame);
  ggiFlush (visu);
}

#endif /* HAVE_LIBGGI */
#ifdef HAVE_LIBSDL

pixel_t *screen_rv = 0;		/* A pointer to the screen buffer associated
				   to the render visual. */

/* screen_rv may be a direct pointer to the hardware video buffer, or
   it may be a pointer to system (mallocated) memory, this depends on
   the display driver (if the videomode were available or needed to be
   emulated etc.).  If screen_rv points directly to hardware video
   it might requires locking. */

SDL_Surface *visu = 0;
int visu_options = SDL_HWPALETTE | SDL_DOUBLEBUF;
char SDL_initialized = 0;
#define SDL_VIDEODRIVER "SDL_VIDEODRIVER"
char *sdl_videodriver = 0;

void set_display_params (const char *str)
{
  sdl_videodriver = strcat_alloc (SDL_VIDEODRIVER "=", str);
  dmsg (D_SYSTEM | D_VIDEO, "put `%s' in environment", sdl_videodriver);
  putenv (sdl_videodriver);
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
    emsg (_("Failed to open visual: %s"), SDL_GetError());

  video_initialized = 1;

  if (SDL_MUSTLOCK (visu))
    dmsg (D_VIDEO, "visual require locking");
  else
    screen_rv = visu->pixels;

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
  dmsg (D_MISC, "free screen buffer");

  if (SDL_initialized) {
    SDL_Quit ();
    SDL_initialized = 0;
  }
  if (sdl_videodriver) {
    /* Remove `SDL_VIDEODRIVER=mumble' from environment before freeing
       sdl_videodriver.  FIXME: This is not const-correct as putenv()
       usually takes a mutable string argument.  */
    putenv (SDL_VIDEODRIVER);
    XFREE0 (sdl_videodriver);
  }
}

void
set_color (unsigned char c, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_Color col;
  col.r = r * 4;
  col.g = g * 4;
  col.b = b * 4;
  dmsg (D_VIDEO, "set color %d=(%d,%d,%d)", c, r, g, b);
  SDL_SetColors (visu, &col, c, 1);
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
vsynchro (const pixel_t *s)
{
  if (SDL_MUSTLOCK (visu))
    SDL_LockSurface (visu);

  screen_rv = visu->pixels;
  copy_display (s, screen_rv, 320);

  if (SDL_MUSTLOCK (visu))
    SDL_UnlockSurface (visu);

  SDL_Flip (visu);		/* can change visu->pixels */
}

void
vsynchro2 (const pixel_t *s1, const pixel_t *s2)
{
  if (SDL_MUSTLOCK (visu))
    SDL_LockSurface (visu);

  screen_rv = visu->pixels;
  copy_display (s1, screen_rv, 160);
  copy_display (s2, screen_rv + 160 * stretch, 160);

  if (SDL_MUSTLOCK (visu))
    SDL_UnlockSurface (visu);

  SDL_Flip (visu);		/* can change visu->pixels */
}


#endif /* HAVE_LIBSDL */
