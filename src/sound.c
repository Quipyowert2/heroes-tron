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
#include <stdlib.h>
#include "sound.h"

#ifdef HAVE_LIBMIKMOD
#include <mikmod.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#include "options.h"
#include "argv.h"
#include "musicfiles.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

MODULE* module;
pthread_t polling_thread;
pthread_mutex_t playing;	/* this mutex is used to tell the polling 
				   thread that it must continue polling ... */
int nth_driver = 0;
char* driver_options = 0;

void
set_volume (void)
{
  if (opt.music)
    md_musicvolume = (13 - opt.music_volume) * 128 / 13;
  else
    md_musicvolume = 0;
  /* 
     This doesn't want to work.  I'm changing the volume of each sample
     as a work around, see event_sfX() in sfx.c.

  if (opt.sfx)
    md_sndfxvolume = (13 - opt.sfx_volume) * 128 / 13;
  else
    md_sndfxvolume = 0;
  */
}

void
halve_volume (void)
{
  md_musicvolume /= 2;
  md_sndfxvolume /= 2;
}

int
init_sound_engine (void)
{
  /* register all the drivers */
  MikMod_RegisterAllDrivers ();

  /* register the all module loader 
     (the user can use something else than .xm) */
  MikMod_RegisterAllLoaders ();

  /* initialize the library */
  md_device = nth_driver;
  md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
  if (mono)
    md_mode &= ~DMODE_STEREO;
  if (bits8)
    md_mode &= ~DMODE_16BITS;
  if (hqmix)
    md_mode |= DMODE_HQMIXER;
  if (MikMod_Init (driver_options?driver_options:"")) {
    fprintf (stderr, "Could not initialize sound, reason: %s\n",
	     MikMod_strerror (MikMod_errno));
    return 1;
  }

  if (MikMod_InitThreads () != 1) {
    fprintf (stderr, "Could not initialize sound, reason: LibMikMod is not thread safe.\n");
    return 1;
  }

  pthread_mutex_init (&playing, 0);
  set_volume ();

  return 0;
}

void
uninit_sound_engine (void)
{
  MikMod_Exit ();
}

void
load_soundtrack (char *ptr)
{  
  module = Player_Load (ptr, 16, 0);
  if (!module) {
    fprintf (stderr, "Could not load %s, reason: %s\n", ptr,
	     MikMod_strerror (MikMod_errno));
  }
}

void
unload_soundtrack (void)
{
  if (!module)
    return;
  pthread_mutex_unlock (&playing);
  pthread_join (polling_thread, 0);
  Player_Stop ();
  //  MikMod_DisableOutput ();
  Player_Free (module);

  module = 0;
  soundtrack_title = 0;
  soundtrack_author = 0;
}

static void *
update_thread (void *arg __attribute__ ((unused)))
{
  while (pthread_mutex_trylock (&playing) == EBUSY) {
    MikMod_Update ();
    usleep (10000);
  }
  pthread_mutex_unlock (&playing);
  return 0;
}

void
play_soundtrack (void)
{
  if (!module)
    return;
  pthread_mutex_lock (&playing);
  MikMod_SetNumVoices (-1, 6);
  /* MikMod_EnableOutput (); */
  Player_Start (module);
  pthread_create (&polling_thread, 0, update_thread, 0);
}

void
print_drivers_list (void)
{
  char* info;
  long engineversion = MikMod_GetVersion();

  MikMod_RegisterAllDrivers ();
  printf ("LibMikMod version %ld.%ld.%ld\n",
	  (engineversion >> 16) & 255,(engineversion >> 8) & 255, engineversion & 255);
  info = MikMod_InfoDriver();
  printf("\nAvailable drivers:\n%s\n", info);
  free (info);
}

/* This function is adapted from from Mikmod 3.1.6 */
static void 
get_int (char *arg, int *value, int min, int max, char* argv0)
{
  char *end = NULL;
  int t = min - 1;
  
  if (arg)
    t = strtol (arg, &end, 10);
  if (end && (!*end) && (t >= min) && (t <= max))
    *value = t;
  else
    fprintf(stderr, 
	    "Argument '%s' out of bounds, must be between %d and %d.\n"
	    "Use '%s --help' for more information.\n",
	    arg?arg:"(not given)", min, max, argv0);
}

void 
decode_sound_options (char* optarg, char* argv0)
{
  /* This is adapted from Mikmod 3.1.6 */
  if (strlen (optarg) > 2) {
    char* opts = strchr (optarg, ',');
    if (opts) {
      *opts=0;
      
      /* numeric driver specification ? */
      if (opts - optarg <= 2)
	get_int (optarg, &nth_driver, 0, 99, argv0);
      else    
	nth_driver = MikMod_DriverFromAlias(optarg);
      if (driver_options)
	free (driver_options);
      driver_options = strdup(opts+1);
    } else  
      nth_driver = MikMod_DriverFromAlias (optarg);
  } else  
    get_int(optarg, &nth_driver, 0, 99, argv0);
}

void 
load_soundtrack_from_alias (char* alias)
{
  sound_track_t* st = get_sound_track_from_alias (alias);

  if (st) {
    load_soundtrack (st->filename);
    soundtrack_title = st->title;
    soundtrack_author = st->author;
  } else {
    module = 0;
    soundtrack_title = 0;
    soundtrack_author = 0;
  }
}

#else // not HAVE_LIBMIKMOD

#ifdef HAVE_LIBSDL_MIXER

#include <stdio.h>
#include <SDL_mixer.h>
#include "argv.h"
#include "musicfiles.h"

static Mix_Music *music = NULL;

int audio_rate;
Uint16 audio_format;
int audio_channels;
int audio_buffers;

void
set_volume (void)
{
}

void
halve_volume (void)
{
}

extern void init_SDL (void);

int
init_sound_engine (void)
{
  audio_rate = (hqmix ? 22050 : 44100);
  audio_format = (bits8 ? AUDIO_S8 : AUDIO_S16);
  audio_channels = (mono ? 1 : 2);
  audio_buffers = 4096;
  
  init_SDL ();
  /* Open the audio device */
  if (Mix_OpenAudio (audio_rate, audio_format, audio_channels, audio_buffers) 
      < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    exit (2);
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", 
	   audio_rate,
	   (audio_format&0xFF),
	   (audio_channels > 1) ? "stereo" : "mono", 
	   audio_buffers );
  }

  /* Set the external music player, if any */
  Mix_SetMusicCMD (getenv ("MUSIC_CMD"));

  return 0;
}

void
uninit_sound_engine (void)
{
  Mix_CloseAudio ();
}

void
load_soundtrack (char *ptr)
{
  music = Mix_LoadMUS(ptr);
  if (!music) {
    fprintf (stderr, "Could not load %s, reason: %s\n", ptr,
	     SDL_GetError ());
  }
}

void
unload_soundtrack (void)
{
  if (music) {
    Mix_FreeMusic (music);
    music = NULL;
  }
}

void
play_soundtrack (void)
{
  if (music)
    Mix_PlayMusic(music, -1);
}

void
print_drivers_list (void)
{
  printf ("Heroes has been compiled with SDL_mixer,"
	  " there is no driver list available.\n");
}

void
decode_sound_options (char* optarg __attribute__ ((unused)), 
		      char* argv0 __attribute__ ((unused)))
{
}

void 
load_soundtrack_from_alias (char* alias)
{
  sound_track_t* st = get_sound_track_from_alias (alias);

  if (st) {
    load_soundtrack (st->filename);
    soundtrack_title = st->title;
    soundtrack_author = st->author;
  } else {
    music = 0;
    soundtrack_title = 0;
    soundtrack_author = 0;
  }
}

#else // not HAVE_LIBSDL_MIXER and not HAVE_LIBMIKMOD

#include <stdio.h>

/* empty implementation */

void
set_volume (void)
{
}

void
halve_volume (void)
{
}

int
init_sound_engine (void)
{
  return 0;
}

void
uninit_sound_engine (void)
{
}

void
load_soundtrack (char *ptr __attribute__ ((unused)))
{
}

void
unload_soundtrack (void)
{
}

void
play_soundtrack (void)
{
}

void
print_drivers_list (void)
{
  printf ("Heroes has been compiled without sound support.\n");
}

void
decode_sound_options (char* optarg __attribute__ ((unused)), 
		      char* argv0 __attribute__ ((unused)))
{
}


void 
load_soundtrack_from_alias (char* alias __attribute__ ((unused)))
{
}

#endif // not HAVE_LIBSDL_MIXER

#endif // not HAVE_MIKMOD
