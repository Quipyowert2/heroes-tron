/*------------------------------------------------------------------.
| Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>           |
|                                                                   |
| This file is part of Heroes.                                      |
|                                                                   |
| Heroes is free software; you can redistribute it and/or modify it |
| under the terms of the GNU General Public License version 2 as    |
| published by the Free Software Foundation.                        |
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
#include "generic_list.h"
#include "hooks.h"

struct a_hook_fun_list_item {
  a_hook_fun fun;
  void *callback_data;
};
typedef struct a_hook_fun_list_item a_hook_fun_list_item;

NEW_LIST(a_hook_fun, a_hook_fun_list_item, MEMCMP_EQUAL, NULL_DESTRUCTOR);

a_hook *hook_list = 0;

void
hook_define (a_hook *hook)
{
  hook->next = hook_list;
  hook_list = hook;
  hook->list = 0;
}

void
hook_run (a_hook *hook, void *hook_data)
{
  a_hook_fun_list l = hook->list;
  while (l) {
    l->car.fun (hook, hook_data, l->car.callback_data);
    l = l->cdr;
  }
}

static a_hook *
hook_find (const char *name)
{
  a_hook *l = hook_list;
  while (l)
    if (strcmp(l->name, name) == 0)
      break;
  return l;
}

void
hook_add_fun (const char *name, a_hook_fun fun, void *callback_data)
{
  a_hook *h = hook_find (name);
  if (h) {
    a_hook_fun_list_item i = { fun, callback_data };
    h->list = a_hook_fun_cons (i, h->list);
  }
}

void *
hook_rem_fun_if (const char *name, a_hook_fun fun, void *ref_data,
		 a_hook_rem_if_fun if_fun)
{
  a_hook *h = hook_find (name);
  if (h) {
    a_hook_fun_list l = h->list;
    while (l) {
      if (l->car.fun == fun)
	if (! if_fun || if_fun (ref_data, l->car.callback_data)) {
	  void *cb = l->car.callback_data;
	  a_hook_fun_delete (&l);
	  return cb;
	}
      l = l->cdr;
    }
  }
  return 0;
}


void *
hook_rem_fun (const char *name, a_hook_fun fun)
{
  return hook_rem_fun_if (name, fun, 0, 0);
}
