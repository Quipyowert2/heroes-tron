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
#include "lvl_priv.h"

const char *
lvl_sound_track (const level_t *lvl)
{
  return lvl->private->sound_track_alias;
}

const char *
lvl_tile_sprite_map_basename (const level_t *lvl)
{
  return lvl->private->tile_sprite_map_basename;
}

void
lvl_start_position (const level_t *lvl, unsigned int player,
		    square_coord_pair_t *coord, dir_t *dir)
{
  if (coord)
    *coord = lvl->private->start_pos[player];
  if (dir)
    *dir = lvl->private->start_dir[player];
}
