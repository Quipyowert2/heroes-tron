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

#include "system.h"
#include "sound.h"
#include "options.h"
#include "argv.h"
#include "musicfiles.h"
#include "debugmsg.h"
#include "errors.h"

char sound_initialized = 0;
char sound_track_loaded = 0;
char sound_track_playing = 0;

#ifdef HAVE_LIBMIKMOD

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
  dmsg (D_SOUND_TRACK, "set volume to %d/128", md_musicvolume);
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
  dmsg (D_SOUND_TRACK|D_SOUND_EFFECT,"halve volume");
}

int
init_sound_engine (void)
{
  if (nosound) {
    nosfx = 1;
    return 0;
  }

  dmsg (D_SYSTEM|D_SOUND_TRACK, "initialize libMikMod");

  /* register all the drivers */
  MikMod_RegisterAllDrivers ();
  dmsg (D_SOUND_TRACK, "libMikMod driver registered");

  /* register the all module loader 
     (the user can use something else than .xm) */
  MikMod_RegisterAllLoaders ();
  dmsg (D_SOUND_TRACK, "libMikMod loader registered");

  /* initialize the library */
  md_device = nth_driver;
  md_mode |= DMODE_SOFT_MUSIC | DMODE_SOFT_SNDFX;
  if (mono)
    md_mode &= ~DMODE_STEREO;
  if (bits8)
    md_mode &= ~DMODE_16BITS;
  if (hqmix)
    md_mode |= DMODE_HQMIXER;
  dmsg (D_SOUND_TRACK, "Opening audio device #%d, with %dbits, %s%s", 
	nth_driver, (md_mode & DMODE_16BITS)?16:8, 
	(md_mode & DMODE_STEREO)?"stereo":"mono",
	(md_mode & DMODE_HQMIXER)?"":", high quality mixer");
  if (driver_options)
    dmsg (D_SOUND_TRACK, "MikMod user options: %s", driver_options);
  if (MikMod_Init (driver_options?driver_options:"")) {
    wmsg ("Could not initialize sound, reason: %s\n"
	  "Disabling sound output (use -S to suppress this message).\n",
	  MikMod_strerror (MikMod_errno));
    nosfx = nosound = 1;
    MikMod_Exit ();
    return 0;
  }

  dmsg (D_SOUND_TRACK,"initialize MikMod thread");
  if (MikMod_InitThreads () != 1) {
    wmsg ("Could not initialize sound, reason: "
	  "LibMikMod is not thread safe.\n"
	  "Disabling sound output (use -S to suppress this message).\n");
    nosfx = nosound = 1;
    MikMod_Exit ();
    return 0;
  }

  sound_initialized = 1;

  pthread_mutex_init (&playing, 0);
  set_volume ();

  return 0;
}

void
uninit_sound_engine (void)
{
  if (sound_initialized) {
    unload_soundtrack ();
    MikMod_Exit ();
    dmsg (D_SOUND_TRACK, "libMikMod exited");
    sound_initialized = 0;
  }
}

void
load_soundtrack (char *ptr)
{
  if (nosound)
    return;
  dmsg (D_FILE|D_SOUND_TRACK,"loading sound track: %s", ptr);
  module = Player_Load (ptr, 16, 0);
  if (!module) {
    wmsg ("Could not load %s, reason: %s\n", ptr,
	  MikMod_strerror (MikMod_errno));
  } else
    sound_track_loaded = 1;
}

void
unload_soundtrack (void)
{
  if (nosound)
    return;
  if (sound_track_playing) {
    dmsg (D_SOUND_TRACK, "joining playing thread");
    pthread_mutex_unlock (&playing);
    pthread_join (polling_thread, 0);
    Player_Stop ();
    sound_track_playing = 0;
  }
  if (sound_track_loaded) {
    dmsg (D_SOUND_TRACK, "unloading sound track");
    Player_Free (module);
    module = 0;
    soundtrack_title = 0;
    soundtrack_author = 0;
    sound_track_loaded = 0;
  }
}

static void *
update_thread (void *arg ATTRIBUTE_UNUSED)
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
  if (nosound)
    return;
  if (!sound_track_loaded)
    return;
  dmsg (D_SOUND_TRACK, "launching sound track playing thread");
  pthread_mutex_lock (&playing);
  MikMod_SetNumVoices (-1, 6);
  /* MikMod_EnableOutput (); */
  Player_Start (module);
  pthread_create (&polling_thread, 0, update_thread, 0);
  sound_track_playing = 1;
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
    wmsg ("Argument '%s' out of bounds, must be between %d and %d.\n"
	  "Use '%s --help' for more information.\n",
	  arg?arg:"(not given)", min, max, argv0);
}

void 
decode_sound_options (char* option_string, char* argv0)
{
  /* This is adapted from Mikmod 3.1.6 */
  if (strlen (option_string) > 2) {
    char* opts = strchr (option_string, ',');
    if (opts) {
      *opts=0;
      
      /* numeric driver specification ? */
      if (opts - option_string <= 2)
	get_int (optarg, &nth_driver, 0, 99, argv0);
      else    
	nth_driver = MikMod_DriverFromAlias(option_string);
      if (driver_options)
	free (driver_options);
      driver_options = strdup (opts+1);
    } else  
      nth_driver = MikMod_DriverFromAlias (option_string);
  } else  
    get_int(option_string, &nth_driver, 0, 99, argv0);
}

void 
load_soundtrack_from_alias (const char* alias)
{
  if (!nosound) {
    sound_track_t* st = get_sound_track_from_alias (alias);

    dmsg (D_SOUND_TRACK, "loading sound track from alias %s", alias);
    
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
}

#else /* not HAVE_LIBMIKMOD */

#ifdef HAVE_LIBSDL_MIXER

static Mix_Music *music = NULL;

int audio_rate = 0;
Uint16 audio_format;
int audio_channels;
int audio_buffers = 0;

void
set_volume (void)
{
  if (opt.music) {
    Mix_VolumeMusic ((13 - opt.music_volume) * MIX_MAX_VOLUME / 13);
    dmsg (D_SOUND_TRACK, "set volume to %d/%d\n",
	  (13 - opt.music_volume) * MIX_MAX_VOLUME / 13, MIX_MAX_VOLUME);
  } else {
    Mix_VolumeMusic (0);
    dmsg (D_SOUND_TRACK, "set volume to 0/%d\n", MIX_MAX_VOLUME);
  }
}

void
halve_volume (void)
{
  if (opt.music) {
    Mix_VolumeMusic ((13 - opt.music_volume) * MIX_MAX_VOLUME / 13 / 2);
    dmsg (D_SOUND_TRACK, "set volume to %d/%d\n",
	  (13 - opt.music_volume) * MIX_MAX_VOLUME / 13 / 2, MIX_MAX_VOLUME);
  } else {
    Mix_VolumeMusic (0);
    dmsg (D_SOUND_TRACK, "set volume to 0/%d\n", MIX_MAX_VOLUME);
  }
}

extern void init_SDL (void);

int
init_sound_engine (void)
{
  if (nosound) {
    nosfx = 1;
    return 0;
  }

  if (!audio_rate)
    audio_rate = (hqmix ? 44100 : 22050);
  audio_format = (bits8 ? AUDIO_S8 : AUDIO_S16);
  audio_channels = (mono ? 1 : 2);
  if (!audio_buffers)
    /* Use small values for audio buffer to reduce the duration between
       the moment where a sample is mixed and the moment where it is heard. */
    audio_buffers = (hqmix ? 2048 : 1024);
  
  init_SDL ();
  /* Open the audio device */
  dmsg (D_SOUND_TRACK, 
	"opening audio at %d Hz %d bit %s, %d bytes audio buffer\n", 
	audio_rate,
	(audio_format&0xFF),
	(audio_channels > 1) ? "stereo" : "mono", 
	audio_buffers);
  if (Mix_OpenAudio (audio_rate, audio_format, audio_channels, audio_buffers) 
      < 0) {
    wmsg ("Couldn't open audio: %s\n"
	  "Disabling sound output (use -S to suppress this message).\n",
	  SDL_GetError());
    nosfx = nosound = 1;
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    dmsg (D_SOUND_TRACK, 
	  "opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", 
	  audio_rate,
	  (audio_format&0xFF),
	  (audio_channels > 1) ? "stereo" : "mono", 
	  audio_buffers);

    sound_initialized = 1;
  }
  set_volume ();

  return 0;
}

void
uninit_sound_engine (void)
{
  if (sound_initialized) {
    unload_soundtrack ();
    Mix_CloseAudio ();
    dmsg (D_SOUND_TRACK, "closed audio");
  }
}

void
load_soundtrack (char *ptr)
{
  if (nosound)
    return;
  dmsg (D_SOUND_TRACK|D_FILE,"loading sound-track: %s", ptr);
  music = Mix_LoadMUS(ptr);
  if (!music) {
    wmsg ("Could not load %s, reason: %s\n", ptr,
	  SDL_GetError ());
  } else
    sound_track_loaded = 1;
}

void
unload_soundtrack (void)
{
  if (nosound)
    return;
  if (sound_track_playing) {
    dmsg (D_SOUND_TRACK, "halt sound track playing");
    Mix_HaltMusic ();
    sound_track_playing = 0;
  }
  if (sound_track_loaded) {
    dmsg (D_SOUND_TRACK, "unload sound track");
    Mix_FreeMusic (music);
    music = NULL;
    sound_track_loaded = 0;
  }
}

void
play_soundtrack (void)
{
  if (nosound)
    return;
  if (sound_track_loaded) {
    dmsg (D_SOUND_TRACK, "start playing sound track");
    Mix_PlayMusic (music, -1);
    sound_track_playing = 1;
  }
}

void
print_drivers_list (void)
{
  wmsg ("Heroes has been compiled with SDL_mixer,"
	" there is no driver list available.\n");
}

void
decode_sound_options (char* optarg, char* argv0)
{
  if (optarg) {
    char* buf = strdup (optarg);
    optarg = strtok (buf, " \t:=,;");
    while (optarg) {
      if (!strcasecmp (optarg, "freq")) {
	optarg = strtok (0, " \t:=,;");
	if (optarg)
	  audio_rate = atol (optarg);
	else 
	  wmsg ("%s: missing parameter for 'freq'", argv0);
      } else if (!strcasecmp (optarg, "buffers")) {
	optarg = strtok (0, " \t:=,;");
	if (optarg)
	  audio_buffers = atol (optarg);
	else 
	  wmsg ("%s: missing parameter for `buffers'", argv0);
      } else
	wmsg ("%s: recognized sound options"
	      "are freq=nnn and buffers=nnn", argv0);
      optarg = strtok (0, " \t:=,;");      
    }
    free (buf);
  }
}

void 
load_soundtrack_from_alias (const char* alias)
{
  if (!nosound) {
    sound_track_t* st = get_sound_track_from_alias (alias);

    dmsg (D_SOUND_TRACK, "loading sound track from alias %s", alias);
    
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
}

#else /* not HAVE_LIBSDL_MIXER and not HAVE_LIBMIKMOD */

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
load_soundtrack (char *ptr ATTRIBUTE_UNUSED)
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
  wmsg ("Heroes has been compiled without sound support.\n");
}

void
decode_sound_options (char* optarg ATTRIBUTE_UNUSED, 
		      char* argv0 ATTRIBUTE_UNUSED)
{
}


void 
load_soundtrack_from_alias (const char* alias ATTRIBUTE_UNUSED)
{
}

#endif /* not HAVE_LIBSDL_MIXER */

#endif /* not HAVE_MIKMOD */
