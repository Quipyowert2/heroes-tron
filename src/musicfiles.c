/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
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
#include "getshline.h"
#include "musicfiles.h"
#include "misc.h"
#include "rsc_files.h"
#include "debugmsg.h"
#include "errors.h"
#include "hash.h"


/* we maintain the sound tracks in two structures: a hash and
   an array.  The hash is used for most lookups, and when building
   the sound tracks database.  The array is initialized once
   the hash has been finished, it is sorted (w.r.t. the alias name)
   and is used by the sound tracks selector in the game (which
   need to be able to load the `next' or `previous' sound track). */

static Hash_table *st_hash;
static sound_track_t **st_array = 0;
static unsigned n_st = 0;	/* number of sound tracks */

/* comparison function for the hash */
static bool
st_equ (const void *left, const void *right)
{
  const sound_track_t *l = left;
  const sound_track_t *r = right;
  return !strcasecmp (l->alias, r->alias);
}

/* comparison function for qsort */
static int
st_cmp (const void *left, const void *right)
{
  const sound_track_t *const *l = left;
  const sound_track_t *const *r = right;
  return strcasecmp ((*l)->alias, (*r)->alias);
}

static unsigned
st_hasher (const void *data, unsigned size)
{
  const sound_track_t *d = data;
  return hash_string (d->alias, size);
}

static void
st_free (void *data)
{
  sound_track_t *st = data;
  free (st->alias);
  free (st->filename);
  free (st->title);
  free (st->author);
  free (st);
}

static sound_track_t*
st_cons (char* alias, char* filename, char* title, char* author)
{
  NEW (sound_track_t, st);
  st->alias = xstrdup (alias);
  st->filename = xstrdup (filename);
  st->title = xstrdup (title);
  st->author = xstrdup (author);
  st->rank = 0;
  return st;
}

void
add_sound_track_cons (char* alias, char* filename, char* title, char* author)
{
  if (!hash_insert (st_hash, st_cons (alias, filename, title, author)))
    xalloc_die ();
}

sound_track_t*
get_sound_track_from_alias (const char* alias)
{
  struct hash_entry *bucket
    = st_hash->bucket + hash_string (alias, st_hash->n_buckets);
  struct hash_entry *cursor;

  assert (bucket < st_hash->bucket_limit);

  if (!bucket->data)
    return 0;

  for (cursor = bucket; cursor; cursor = cursor->next)
    if (!strcasecmp (((sound_track_t*)(cursor->data))->alias, alias))
      return cursor->data;

  return 0;
}

sound_track_t*
get_sound_track_from_rank (unsigned rank)
{
  return st_array[rank];
}


static char*
dir_name (const char* filename)
{
  char* pos = strrchr (filename, '/');
  char* res;
  if (pos == 0)
    return 0;
  XMALLOC_ARRAY (res, pos - filename + 2);
  strncpy (res, filename, pos - filename + 1);
  res[pos - filename + 1] = 0;
  return res;
}

int
read_sound_config_file (char* filename)
{
  FILE* fs;
  char* buf = 0;
  size_t bufsize = 0;
  int firstline = 0, endline = 0;
  char* expfilename = rsc_expand (filename);
  char* dir = dir_name (expfilename);

  dmsg (D_SECTION|D_FILE,"reading sound config file: %s ...", expfilename);

  fs = fopen (expfilename, "r");

  if (!fs) {
    dmsg (D_SECTION|D_FILE,"... could not open.");
    dperror ("fopen");
    free (expfilename);
    free (dir);
    return 0;
  }

  while (getshline_numbered
	 (&firstline, &endline, &buf, &bufsize, fs) != -1) {
    char* alias = strtok (buf, ":\n");
    char* file  = strtok (0, ":\n");
    char* title  = strtok (0, ":\n");
    char* author  = strtok (0, "\n");
    if (!alias || !alias[0])
      wmsg ("%s:%d: missing alias name\n", filename, firstline);
    else if (!file || !file[0])
      wmsg ("%s:%d: missing file name\n", filename, firstline);
    else if (!title || !title[0])
      wmsg ("%s:%d: missing title\n", filename, firstline);
    else if (!author || !author[0])
      wmsg ("%s:%d: missing author\n", filename, firstline);
    else {
      if (dir && file[0] != '/') {
	char* tmp = strcat_alloc (dir, file);
	add_sound_track_cons (alias, tmp, title, author);
	free (tmp);
      } else
	add_sound_track_cons (alias, file, title, author);
    }
  }
  fclose (fs);
  free (buf);
  free (dir);
  free (expfilename);
  dmsg (D_SECTION|D_FILE,"... done.");

  return 0;
}

void
init_sound_track_list (void)
{
  dmsg (D_MISC, "initialize sound track hash");

  st_hash = hash_initialize (17, NULL, st_hasher, st_equ, st_free);
  if (!st_hash)
    xalloc_die ();
}

void
uninit_sound_track_list (void)
{
  dmsg (D_MISC, "free sound track hash");
  hash_free (st_hash);
  XFREE0 (st_array);
  n_st = 0;
}

void
print_sound_track_list (void)
{
  unsigned pos;
  for (pos = 0; pos < n_st; ++pos)
    printf ("%s:%s:%s:%s\n",
	    st_array[pos]->alias, st_array[pos]->filename,
	    st_array[pos]->title, st_array[pos]->author);
}

void
print_sound_track_list_stat (void)
{
  hash_print_statistics (st_hash, stdout);
}

void
freeze_sound_track_list (void)
{
  sound_track_t *s;
  sound_track_t **p;
  unsigned pos;

  n_st = hash_get_n_entries (st_hash);
  if (!n_st)
    return;

  /* create an array of all existing aliases */

  XMALLOC_ARRAY (st_array, n_st);
  pos = 0;
  for (s = hash_get_first (st_hash); s; s = hash_get_next (st_hash, s))
    st_array[pos++] = s;

  assert (pos == n_st);

  /* sort it */
  qsort (st_array, n_st, sizeof (*st_array), st_cmp);

  /* number each sound track */
  p = st_array;
  for (pos = 0; pos < n_st; ++pos)
    (*p++)->rank = pos;
}
