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
#include "opponents.h"
#include "generic_list.h"

NEW_LIST(an_opponent, an_opponent_sig*, STD_EQUAL, NULL_DESTRUCTOR);

static an_opponent_list opponent_list = 0;

void
opponent_register (an_opponent_sig *opp)
{
  an_opponent_push (&opponent_list, opp);
}

void
opponent_unregister (an_opponent_sig *opp)
{
  an_opponent_list *l = &opponent_list;
  while (*l && (*l)->car != opp)
    l = &(*l)->cdr;
  if (*l)
    an_opponent_delete (l);
}

an_opponent_sig *
opponent_get_random (a_game_mode mode)
{
  an_opponent_list l = opponent_list;
  an_opponent_type type = (1 << mode);
  /* Count the number of opponent available for MODE.  */
  int n = 0;
  while (l) {
    if (l->car->type & type)
      ++n;
    l = l->cdr;
  }

  if (!n)
    return 0;

  /* Select one of these opponents randomly.  */
  n = rand () % n;
  l = opponent_list;
  while (l) {
    if (l->car->type & type)
      if (n-- == 0)
	return l->car;
    l = l->cdr;
  }
  assert (0);
  return 0;
}
