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
#include "debugmsg.h"

struct a_hook_fun_list {
  a_hook_fun_list *next;
  a_hook_fun fun;
  void *callback_data;
};

a_hook *hook_list = 0;

void
hook_define (a_hook *hook)
{
  dmsg (D_HOOKS, "defining hook `%s'", hook->name);
  hook->next = hook_list;
  hook_list = hook;
  hook->list = 0;
}

void
hook_define_many (a_hook *hook, int size)
{
  while (size--) {
    hook_define (hook);
    ++hook;
  }
}

void
hook_undefine (a_hook *hook)
{
  dmsg (D_HOOKS, "undefining hook `%s'", hook->name);

  /* Maybe HOOK comes first in the list.  */
  if (hook_list == hook) {
    hook_list = hook->next;
  } else {
    /* Otherwise we need to find which hook is pointing to this one.  */
    a_hook *h = hook_list;
    while (h && h->next != hook)
      h = h->next;
    /* Did we found it?  */
    if (h)
      h->next = hook->next;
  }
  /* Wipe out the registered functions.  */
  {
    a_hook_fun_list *l = hook->list;
    while (l) {
      a_hook_fun_list *t = l->next;
      free (l);
      l = t;
    }
  }
}

void
hook_undefine_many (a_hook *hook, int size)
{
  while (size--) {
    hook_undefine (hook);
    ++hook;
  }
}

void
hook_run (a_hook *hook, void *hook_data)
{
  a_hook_fun_list *l = hook->list;
  dmsg (D_HOOKS, "running hook `%s'", hook->name);
  while (l) {
    dmsg (D_HOOKS, "running functions `%p'", l->fun);
    l->fun (hook, hook_data, l->callback_data);
    l = l->next;
  }
}

static a_hook *
hook_find (const char *name)
{
  a_hook *l = hook_list;
  while (l) {
    if (strcmp(l->name, name) == 0)
      break;
    l = l->next;
  }
  return l;
}

void
hook_add_fun (const char *name, a_hook_fun fun, void *callback_data)
{
  a_hook *h = hook_find (name);
  if (h) {
    NEW(a_hook_fun_list, i);
    dmsg (D_HOOKS, "adding function %p to hook `%s'", fun, name);
    i->fun = fun;
    i->callback_data = callback_data;
    i->next = h->list;
    h->list = i;
  }
}

void *
hook_rem_fun_if (const char *name, a_hook_fun fun, void *ref_data,
		 a_hook_rem_if_fun if_fun)
{
  a_hook *h = hook_find (name);
  if (h) {
    a_hook_fun_list **l = &h->list;
    while (*l) {
      if ((*l)->fun == fun)
	if (! if_fun || if_fun (ref_data, (*l)->callback_data)) {
	  void *cb = (*l)->callback_data;
	  a_hook_fun_list *t = *l;
	  dmsg (D_HOOKS, "removing function %p from hook `%s'", fun, name);
	  (*l) = (*l)->next;
	  free (t);
	  return cb;
	}
      l = &(*l)->next;
    }
  }
  return 0;
}


void *
hook_rem_fun (const char *name, a_hook_fun fun)
{
  return hook_rem_fun_if (name, fun, 0, 0);
}
