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

#ifndef HEROES__HOOKS__H
#define HEROES__HOOKS__H

/** -- BEGIN PUBLIC -- **/

/* Plug-ins can "hook" some functions which must be called back
   whenever a "hook point" is encountered.

   This implementation is heavily inspired from the one found in Zsh.
*/

typedef struct a_hook a_hook;
typedef struct a_hook_fun_list a_hook_fun_list;
typedef void (*a_hook_fun)(const a_hook *hook,
			   void *hook_data,
			   void *callback_data);

struct a_hook {
  a_hook *next;
  const char *name;
  a_hook_fun_list *list;
};

#define HOOK_DEF(name)  { 0, name, 0 }

/* The code which provide a hook should define a hook structure as
   follow

   static a_hook sample_hook = HOOK_DEF ("sample");

   and call hook_define to make this hook available to this other part
   of the program:

   hook_define (&sample_hook);

   Whenevert the point where this hook should be run is encountered,
   the hook_run function should be used:

   hook_run (&sample_hook, 0);

   The second argument of hook_run may hold additional data which will
   be passed to all functions run for this hook.  (It is up to the
   provider and user of a hook to use the right convention to access
   the supplied data).  */
void hook_define (a_hook *hook);
void hook_run (a_hook *hook, void *hook_data);

/* Same a hook_define, but works on an array of hooks.  */
void hook_define_many (a_hook *hook, int size);

void hook_undefine (a_hook *hook);
void hook_undefine_many (a_hook *hook, int size);


/* One can subscribe to a hook using the hook_add_fun function, and
   unsubscribe using the hook_rem_fun function.

   The functions added to a hook should have the following prototype:

   void
   sample_hook_fun (const a_hook *hook, void *hook_data, void *callback_data)
   {
     ...
   }

   The HOOK pointer points to the hook for which is function is being
   called back.  This is almost useless, except to get the name
   of this hook (hook->name).

   HOOK_DATA might point to additional data provided by the hook provider,
   its interpretation is hook-dependant.

   CALLBACK_DATA is a pointer that was supplied when the function was
   added registered for the hook being run. */
void hook_add_fun (const char *name, a_hook_fun fun, void *callback_data);

/* This remove the _first_ occurence of fun from the list of functions
   registered for a hook.  The returned pointer is the CALLBACK_DATA
   supplied to hook_add_fun.  */
void *hook_rem_fun (const char *name, a_hook_fun fun);

/* Remove an occurence of FUN from the list of functions registered
   for a hook.  The occurence removed is the first occurence for which
   IF_FUN returns true.

   This is usually used to remove a function whose CALLBACK_DATA (as
   indicated to hook_add_fun) matches something indicated by REF_DATA
   (passed to hook_rem_fun_if).  */

typedef bool (*a_hook_rem_if_fun)(void *ref_data, void *callback_data);
void *hook_rem_fun_if (const char *name, a_hook_fun fun, void *ref_data,
		       a_hook_rem_if_fun if_fun);

/** -- END PUBLIC -- **/

#endif /* HEROES__HOOKS__H */
