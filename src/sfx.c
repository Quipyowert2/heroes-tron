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
#include "sfx.h"
#include "errors.h"

#if defined HAVE_LIBMIKMOD || defined HAVE_LIBSDL_MIXER

#include "options.h"
#include "misc.h"
#include "argv.h"
#include "rsc_files.h"
#include "debugmsg.h"

#define max_events 151

typedef char* filename_t;

static filename_t *sfx_names;
static char *sfx_loaded;
static int *play_handles;
static int event_handle[max_events];
static int max_sfx = 0;

#ifdef HAVE_LIBMIKMOD

static struct SAMPLE **sfx_handles;

static struct SAMPLE*
_load_sfx (char* file)
{
  struct SAMPLE* tmp;

  tmp = Sample_Load (file);
  if (tmp)
    tmp->panning = (PAN_RIGHT + PAN_LEFT) / 2;
  return tmp;
}

static void
_free_sfx (struct SAMPLE* sfx)
{
  Sample_Free (sfx);
}

static void
_play_sfx (struct SAMPLE* sfx)
{
    /* set the sample volume */
    sfx->volume = (13 - opt.sfx_volume) * 64 / 13;
    Sample_Play (sfx, 0, 0);
}

#endif /* HAVE_LIBMIKMOD */

#ifdef HAVE_LIBSDL_MIXER

static Mix_Chunk **sfx_handles;

static Mix_Chunk*
_load_sfx (char* file)
{
  return Mix_LoadWAV (file);
}

static void
_free_sfx (Mix_Chunk* sfx)
{
  Mix_FreeChunk (sfx);
}

static void
_play_sfx (Mix_Chunk* sfx)
{
    /* set the sample volume */
    sfx->volume = (13 - opt.sfx_volume) * MIX_MAX_VOLUME / 13;
    Mix_PlayChannel (-1, sfx, 0);
}

#endif /* HAVE_LIBSDL_MIXER */

static void
remove_comments (char *str)
{
  int i;
  for (i = 0;
       (str[i] != ';') && (str[i] != 0) && (str[i] != 13) && (str[i] != 10);
       i++);
  i--;
  for (; str[i] == ' '; i--);
  str[i + 1] = 0;
}

static char *
forward_spaces (char *tmp)
{
  while (*tmp == ' ')
    tmp++;
  return (tmp);
}

char
read_sfx_conf (void)
{
  FILE *fconf;
  char tmp[256];
  char *tmpptr = (char *) tmp;
  char *tmpptr2;
  char c;
  int nbr;
  char *sfxdir = get_rsc_file ("sfx-dir");

  if (sfxdir == 0)
    nosfx = 1;
  if (nosfx)
    return 0;

  {
    char* conf = get_rsc_file ("sfx-conf-txt");
    if (conf == 0) {
      nosfx = 1;
      return 0;
    }
    dmsg (D_SOUND_EFFECT|D_FILE, "reading sound effects config: %s", conf);
    if ((*conf == 0) || (fconf = fopen (conf, "rt")) == NULL) {
      if (*conf != 0)
	dperror ("fopen");
      wmsg (_("Cannot open %s, disabling sound-effects\n"
	      "(run with -X to supress this message)."), conf);
      nosfx = 1;
      free (conf);
      return 0;
    }
    free (conf);
  }
  while (fgets (tmpptr, 256, fconf) != NULL) {
    c = toupper (tmp[0]);
    remove_comments (tmpptr);
    if (c == 'M') {
      if (sfx_names != NULL)
	return (-1);
      tmpptr = forward_spaces (tmpptr + 1);
      max_sfx = atol (tmpptr);
      XCALLOC_ARRAY (sfx_names, max_sfx);
      XCALLOC_ARRAY (sfx_loaded, max_sfx);
      XCALLOC_ARRAY (sfx_handles, max_sfx);
      XCALLOC_ARRAY (play_handles, max_sfx);
    } else if (c == 'F') {
      tmpptr = forward_spaces (tmpptr + 1);
      tmpptr2 = strchr (tmpptr, ' ');
      *tmpptr2++ = 0;
      nbr = atol (tmpptr);
      tmpptr2 = forward_spaces (tmpptr2);
      if (nbr < max_sfx)
	sfx_names[nbr] = strcat_alloc (sfxdir, tmpptr2);
    } else if (c == 'E') {
      tmpptr = forward_spaces (tmpptr + 1);
      tmpptr2 = strchr (tmpptr, ' ');
      *tmpptr2++ = 0;
      nbr = atol (tmpptr);
      tmpptr2 = forward_spaces (tmpptr2);
      if (nbr < max_events)
	event_handle[nbr] = atol (tmpptr2);
    }
    tmpptr = tmp;
  }
  fclose (fconf);
  free (sfxdir);
  return 0;
}

void
close_sfx_handle (void)
{
  int i;

  if (nosfx)
    return;
  free (play_handles);
  free (sfx_handles);
  free (sfx_loaded);
  for (i = 0; i < max_sfx; ++i)
    XFREE (sfx_names[i]);
  free (sfx_names);
}

#define mark_sfx(n) { 				\
  assert (event_handle[( n )] < max_sfx); 	\
  sfx_loaded[event_handle[( n )]] = 1;		\
}

static void
std_sfx_set (void)
{
  int i;
  mark_sfx (1);
  for (i = 20; i < 36; i++)
    mark_sfx (i);
  for (i = 37; i < 70; i++)
    mark_sfx (i);
  for (i = 85; i < 90; i++)
    mark_sfx (i);
  for (i = 120; i < 130; i++)
    mark_sfx (i);
  for (i = 140; i < 150; i++)
    mark_sfx (i);
}

void
free_all_sfx (void)
{
  int i;
  if (nosfx)
    return;

  dmsg (D_SOUND_EFFECT, "freeing all sound effects");
  for (i = 1; i < max_sfx; i++)
    if (sfx_loaded[i]) {
      _free_sfx (sfx_handles[i]);
      sfx_loaded[i] = 0;
    }
}

void
load_sfx_mode (signed char mode)
{
  int i;

  if (nosfx)
    return;

  dmsg (D_SOUND_EFFECT,"switching to sound effect mode %d", mode);

  free_all_sfx ();

/* tagging sfx to load */

  switch (mode) {
    /* menus */
  case -1:
    for (i = 1; i < 20; i++)
      mark_sfx (i);
    for (i = 70; i < 80; i++)
      mark_sfx (i);
    for (i = 110; i < 120; i++)
      mark_sfx (i);
    for (i = 130; i < 140; i++)
      mark_sfx (i);
    break;

  /*quest */
  case 0:
    std_sfx_set ();
    break;

  /*death */
  case 1:
    std_sfx_set ();
    break;

    /*kilem */
  case 2:
    for (i = 90; i < 100; i++)
      mark_sfx (i);
    std_sfx_set ();
    break;

  /*tca$h */
  case 3:
    mark_sfx (36);
    mark_sfx (80);
    mark_sfx (81);
    std_sfx_set ();
    break;

  /*color */
  case 4:
    for (i = 100; i < 106; i++)
      mark_sfx (i);
    std_sfx_set ();
    break;

  default:
    assert (0 /* unknown sfx-mode */ );
    break;
  }

/* loading tagged sfx */

  for (i = 1; i < max_sfx; i++)
    if (sfx_loaded[i]) {
      dmsg (D_FILE|D_SOUND_EFFECT,"loading sound effect: %s", sfx_names[i]);
      if (!(sfx_handles[i] = _load_sfx (sfx_names[i]))) {
	emsg (_("Unable to load sample %s"), sfx_names[i]);
      }
    }
}


void
event_sfx (int event)
{
  if (nosfx)
    return;
  dmsg (D_SOUND_EFFECT, "sound effect event %d", event);
  assert (event < max_events);
  assert (event_handle[event] < max_sfx);
  if (event_handle[event] != 0) {
    assert (sfx_loaded[event_handle[event]]);
    if (opt.sfx)
      _play_sfx (sfx_handles[event_handle[event]]);
  }
}

#else /* !HAVE_LIBMIKMOD and !HAVE_LIBSDL_MIXER */

char
read_sfx_conf (void)
{
  return 0;
}

void close_sfx_handle (void)
{
}

void load_sfx_mode (signed char mode ATTRIBUTE_UNUSED)
{
}

void free_all_sfx (void)
{
}

void event_sfx (int event ATTRIBUTE_UNUSED)
{
}

#endif
