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


#include "sfx.h"
#include "config.h"

#ifdef HAVE_LIBMIKMOD
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#else
#  include <strings.h>
#endif
#include "errors.h"
#include "options.h"
#include "misc.h"
#include <mikmod.h>
#include "argv.h"
#include "rsc_files.h"
#ifdef HAVE_DMALLOC
#include <dmalloc.h>
#endif

#define max_events 151

typedef char* filename_t;

static filename_t *sfx_names;
static char *sfx_loaded;
static struct SAMPLE **sfx_handles;
static int *play_handles;
static int event_handle[max_events];
static int max_sfx = 0;

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
    if ((*conf == 0) || (fconf = fopen (conf, "rt")) == NULL) {
      fprintf(stderr, "Cannot open %s, disabling sound-effects\n"
	      "(run with -X to supress this message).\n", conf);
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

      sfx_names = calloc (max_sfx, sizeof (filename_t));
      sfx_loaded = calloc (max_sfx, sizeof (char));
      sfx_handles = calloc (max_sfx, sizeof (*sfx_handles));
      play_handles = calloc (max_sfx, sizeof (int));
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
    if (sfx_names[i])
      free (sfx_names[i]);
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
  for (i = 1; i < max_sfx; i++)
    if (sfx_loaded[i]) {
      Sample_Free (sfx_handles[i]);
      sfx_loaded[i] = 0;
    }
}

void
load_sfx_mode (signed char mode)
{
  int i;

  if (nosfx)
    return;

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
      if (!(sfx_handles[i] = Sample_Load (sfx_names[i]))) {
	fprintf(stderr,"%s :",sfx_names[i]);
	fatal_error ("Unable to load that sample.");
      } else {
	sfx_handles[i]->panning = (PAN_RIGHT + PAN_LEFT) / 2;
      }
    }
}


void
event_sfx (int event)
{
  if (nosfx)
    return;
  assert (event < max_events);
  assert (event_handle[event] < max_sfx);
  if (event_handle[event] != 0) {
    struct SAMPLE* i = sfx_handles[event_handle[event]];
    assert (sfx_loaded[event_handle[event]]);
    if (opt.sfx) {
      /* set the sample volume */
      i->volume = (13 - opt.sfx_volume) * 64 / 13;
      Sample_Play (i, 0, 0);
    }
  }
}

#else /* !HAVE_LIBMIKMOD */

char 
read_sfx_conf (void)
{
  return 0;
}

void close_sfx_handle (void)
{
}

void load_sfx_mode (signed char mode __attribute__ ((unused)))
{
}

void free_all_sfx (void)
{
}

void event_sfx (int event __attribute__ ((unused)))
{
}

#endif
