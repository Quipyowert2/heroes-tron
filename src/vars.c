/*------------------------------------------------------------------.
| Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
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
#include "hash.h"
#include "vars.h"

typedef struct var_entry_t var_entry_t;
struct var_entry_t {
  char *name;
  char *value;
};

Hash_table *var_hash;

static bool
var_equ (const void *left, const void *right)
{
  const var_entry_t *l = left;
  const var_entry_t *r = right;
  return !strcasecmp (l->name, r->name);
}

static unsigned
var_hasher (const void *data, unsigned size)
{
  const var_entry_t *l = data;
  return hash_string (l->name, size);
}

static void
var_free (void *data)
{
  var_entry_t *l = data;
  XFREE (l->name);
  XFREE (l->value);
  free (l);
}

static var_entry_t *
var_cons (const char *name, const char *value)
{
  NEW (var_entry_t, v);
  v->name = name ? xstrdup (name) : 0;
  v->value = value ? xstrdup (value) : 0;
  return v;
}

void
var_initialize (void)
{
  var_hash = hash_initialize (17, NULL, var_hasher, var_equ, var_free);
  if (!var_hash)
    xalloc_die ();
}

void
var_uninitialize (void)
{
  hash_free (var_hash);
}

void
var_define (const char *name, const char *value)
{
  if (!hash_insert (var_hash, var_cons (name, value)))
    xalloc_die ();
}

const char *
var_get_value (const char *name)
{
  struct hash_entry *bucket
    = var_hash->bucket + hash_string (name, var_hash->n_buckets);
  struct hash_entry *cursor;

  assert (bucket < var_hash->bucket_limit);

  if (!bucket->data)
    return 0;

  for (cursor = bucket; cursor; cursor = cursor->next)
    if (!strcasecmp (((var_entry_t*)(cursor->data))->name, name))
      return cursor->data;

  return 0;
}

void
var_print_all (void)
{
  var_entry_t *cur = hash_get_first (var_hash);
  while (cur) {
    printf ("%s = %s\n", cur->name, cur->value);
    cur = hash_get_next (var_hash, cur);
  }
}
