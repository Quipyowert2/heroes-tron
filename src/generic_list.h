/*------------------------------------------------------------------------.
| Copyright (C) 2000 Alexandre Duret-Lutz <duret_g@epita.fr>              |
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

/*
 * This is a generic list.  Since `void*' is evil, all that file is
 * written as a macro that will instanciate the list structures and
 * functions for a given type.
 *
 * Use this as you would use a generic Ada package.  Where, in Ada, you would
 * write:
 *   package Foo is new List(Integer);
 * you will write, in C:
 *   NEW_LIST(Foo,int);
 *
 * Once you have called that macro, each function/structure is available
 * with a `Foo_' prefix.  e.g.
 *   Foo_list_t = Foo_cons (12, Foo_cons (5, 0));
 */

#define NEW_LIST(PREFIX,TYPE)						\
									\
struct PREFIX##_list_s {						\
  TYPE car;								\
  struct PREFIX##_list_s* cdr;						\
};									\
									\
typedef struct PREFIX##_list_s* PREFIX##_list_t;			\
									\
/* declarations */							\
									\
PREFIX##_list_t PREFIX##_cons   (TYPE value, PREFIX##_list_t tail);	\
void            PREFIX##_delete (PREFIX##_list_t* list);		\
PREFIX##_list_t PREFIX##_member (PREFIX##_list_t list, TYPE value);	\
void            PREFIX##_push   (PREFIX##_list_t* list, TYPE value);	\
TYPE            PREFIX##_pop    (PREFIX##_list_t* list);		\
void		PREFIX##_clear  (PREFIX##_list_t* list);		\
									\
/* definitions */							\
									\
PREFIX##_list_t 							\
PREFIX##_cons (TYPE value, PREFIX##_list_t tail)			\
{									\
  PREFIX##_list_t result = malloc (sizeof (*result));			\
  result->car = value;							\
  result->cdr = tail;							\
  return result;							\
}									\
									\
void 									\
PREFIX##_delete (PREFIX##_list_t* list)					\
{									\
  PREFIX##_list_t result = *list;					\
  *list = (*list)->cdr;							\
  free (result);							\
}									\
									\
PREFIX##_list_t 							\
PREFIX##_member (PREFIX##_list_t list, TYPE value)			\
{									\
  while (list) {							\
    if (list->car == value)						\
      return list;							\
    list = list->cdr;							\
  }									\
  return 0;								\
}									\
									\
void									\
PREFIX##_push (PREFIX##_list_t* list, TYPE value)			\
{									\
  *list = PREFIX##_cons (value, *list);					\
}									\
									\
TYPE									\
PREFIX##_pop (PREFIX##_list_t* list)					\
{									\
  TYPE result = (*list)->car;						\
  PREFIX##_delete (list);						\
  return result;							\
}									\
									\
void									\
PREFIX##_clear (PREFIX##_list_t* list)					\
{									\
  PREFIX##_list_t next;							\
									\
  while (*list) {							\
    next = (*list)->cdr;						\
    free (*list);							\
    *list = next;							\
  }									\
}
