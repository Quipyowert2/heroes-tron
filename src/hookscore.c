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
#include "hookscore.h"

a_hook hooks_core[] = {
  /* These hooks are called whenever a bonus is added or removed from the
     board.  The HOOK_DATA pointer, as received by the functions registered
     for this hook, is a pointer to a `a_tile_index' variable holding the
     position of the changed (removed or added) bonus.  */
  HOOK_DEF ("bonus-add"),
  HOOK_DEF ("bonus-rem"),
};


void
hooks_core_initialize (void)
{
  hook_define_many (hooks_core, sizeof(hooks_core)/sizeof(a_hook));
}

void
hooks_core_finalize (void)
{
  hook_undefine_many (hooks_core, sizeof(hooks_core)/sizeof(a_hook));
}
