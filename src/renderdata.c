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

#include "common.h"
#include "renderdata.h"
#include "errors.h"
#include "debugmsg.h"
#include "structs.h"
#include "const.h"

bg_data_t* bg_data = 0;

void
uninit_render_data (void)
{
  dmsg (D_SECTION, "Uninitialize rendering data");

  free (bg_data);
  bg_data = 0;
}

void
init_render_data (void)
{
  unsigned pos;
  unsigned max_pos = map_info.xt * map_info.yt;

  dmsg (D_SECTION, "Initialize rendering data");

  bg_data = malloc (max_pos * sizeof (*bg_data));
  if (!bg_data)
    emsg ("Not enough memory, cannot allocate bg_data.");

  /* initialize background tile information */
    
  for (pos = 0; pos < max_pos; ++pos) {
    bg_data[pos].source = level_map[pos].number + tile_set_img.buffer;
    if (level_map[pos].type == t_anim) {
      /* Cyclic animation */
      bg_data[pos].kind = A_LOOP;
      bg_data[pos].anim_speed = level_map[pos].info.anim.speed + 1;
      bg_data[pos].anim_frames = level_map[pos].info.anim.frame_nbr + 1;
    } else if (((level_map[pos].info.param[4] & 0xf0) != 0)
	       && (level_map[pos].type == t_speed 
		   || level_map[pos].type == t_boom
		   || level_map[pos].type == t_stop
		   || level_map[pos].type == t_ice
		   || level_map[pos].type == t_outway
		   || level_map[pos].type == t_dust)) {
      /* Bounced animation */
      bg_data[pos].kind = A_PINGPONG;
      bg_data[pos].anim_speed = (level_map[pos].info.param[4] & 0x0f) + 1;
      bg_data[pos].anim_frames = (level_map[pos].info.param[4] & 0xf0) >> 4;
    } else {
      /* Static tile */
      bg_data[pos].kind = A_NONE;
      bg_data[pos].anim_speed = 0;
      bg_data[pos].anim_frames = 0;
    }
  }
}
