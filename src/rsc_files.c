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
#include "rsc_files.h"
#include "rsc_files_hash.h"
#include "errors.h"
#include "debugmsg.h"

static void
check_rsc_spelling_error (const char *rsc_name)
{
  const char *approx = get_rsc_approx (rsc_name);
  if (approx)
    wmsg ("%s: no such resource, possible misspelling of '%s'.",
	  rsc_name, approx);
  else
    wmsg ("%s: no such resource.", rsc_name);
}

int
set_rsc_file (const char* rsc_name, const char* file_name)
{
  struct rsc_file* res = in_rsc_set (rsc_name, strlen(rsc_name));

  if (res == 0) {
    check_rsc_spelling_error (rsc_name);
    return 1;
  }
  dmsg (D_RESOURCE, "set resource $(%s)=%s", rsc_name, file_name);
  if (res->modified_value)
    free (res->modified_value);
  res->modified_value = strdup (file_name);
  return 0;
}

/*  Search value for strings of the form `$(name)' to expand.  */
char*
rsc_expand (char* value)
{
  int size = strlen (value) + 1;
  char* result = malloc (size);
  int dest;

  for (dest = 0; *value; ++value)
    if (*value != '$' || value[1] != '(')
      result[dest++] = *value;
    else {
      char* end;

      value += 2;
      /* look for the closing parenthethis */
      for (end = value; *end && *end != ')'; ++end)
	/* NOP */;

      if (*end == 0) {		/* no closing parenthethis found */
	free (result);
	return 0;
      }
      *end = 0;

      /* insert the expanded value into the result string */
      {
	char* expanded;

	expanded = get_rsc_file (value);
	if (expanded) {
	  char* src;

	  size += strlen (expanded) - (end - value + 3);
	  result = realloc (result, size);
	  for (src = expanded; *src;)
	    result[dest++] = *src++;
	  free (expanded);
	}
	value = end;
      }
    }
  result[dest] = 0;
  return result;
}

char*
get_rsc_file (const char* rsc_name)
{
  char* result;
  char* tmp;
  struct rsc_file* res = in_rsc_set (rsc_name, strlen(rsc_name));

  if (res == 0) {
    check_rsc_spelling_error (rsc_name);
    return 0;
  }
  if (res->expanded)		/* prevent infinite recursion */
    return 0;
  res->expanded = 1;

  /* strdup the value, rsc_expand will modify it */
  tmp = strdup (res->modified_value ? res->modified_value : res->value);
  dmsg (D_RESOURCE, "get resource $(%s)=%s", rsc_name, tmp);
  result = rsc_expand (tmp);
  dmsg (D_RESOURCE, "expanded resource $(%s)=%s", rsc_name, result);
  free (tmp);
  res->expanded = 0;
  return result;
}

char*
get_non_null_rsc_file (const char* rsc_name)
{
  char* tmp = get_rsc_file (rsc_name);
  if (tmp == 0)
    emsg ("%s: null resource", rsc_name);
  return tmp;
}
