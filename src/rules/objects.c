/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001, 2002  Alexandre Duret-Lutz      |
|                                          <duret_g@epita.fr>       |
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
#include "statepriv.h"

void
add_color (a_level_state *state, const a_level *lvl, bool allow_clocks)
{
  a_square_index d;
  unsigned char b;
  do {
    d = rand () % lvl->square_count;
  }
  while (state->square_object[d] != -1);
  if (allow_clocks && (rand () % 40 == 0))
    b = 16;
  else
    b = rand () % 5;
  if ((rand () & 3) == 0)
    b |= 8;
  state->square_object[d] = b;
}

void
add_cash (a_level_state *state, const a_level *lvl, bool allow_clocks)
{
  a_square_index d;
  unsigned char b = 0;
  do {
    d = rand () % lvl->square_count;
  }
  while (state->square_object[d] != -1);
  if (allow_clocks && (rand () % 40 == 0))
    b = 15;
  state->square_object[d] = b;
}
