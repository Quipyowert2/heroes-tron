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
#include "renderdata.h"
#include "errors.h"
#include "debugmsg.h"
#include "structs.h"
#include "const.h"

bg_data_t *bg_data = 0;
fg_data_t *fg_data = 0;

int tile_set_size = 0;		/* number of tiles in the tile set */
rleprog_t **tile_sprites;	/* An array as wide as the tile set,
				   which might contains pointer to
				   the rleprog_t for a tile (usefull when
				   that tile is used as a sprite) */
static void
init_tile_sprites (void)
{
  tile_set_size = (tile_set_img.width / 24) * 10;
  XCALLOC_ARRAY (tile_sprites, tile_set_size);
}

static void
uninit_tile_sprites (void)
{
  while (tile_set_size--)
    XFREE0 (tile_sprites[tile_set_size]);
  XFREE0 (tile_sprites);
}

static rleprog_t *
get_tile_sprite (unsigned int offset)
{
  /* convert an offset-in-image, into a tile-number */
  int tile_row = offset / (tile_set_img.width * 20);
  int tile_col = (offset % (tile_set_img.width * 20)) / 24;
  int tile_pos = tile_row * (tile_set_img.width / 24) + tile_col;

  /* don't recompile the sprite if it's already done */
  if (!tile_sprites[tile_pos])
    tile_sprites[tile_pos] =
      compile_rleprog (tile_set_img.buffer + offset, 0, 20, 24,
		       tile_set_img.width, xbuf);

  return tile_sprites[tile_pos];
}

void
uninit_render_data (void)
{
  dmsg (D_SECTION, "Uninitialize rendering data");

  uninit_tile_sprites ();
  XFREE0 (bg_data);
  XFREE0 (fg_data);
}

void
init_render_data (void)
{
  unsigned pos;
  unsigned max_pos = map_info.xt * map_info.yt;

  dmsg (D_SECTION, "Initialize rendering data");

  XMALLOC_ARRAY (bg_data, max_pos);
  XMALLOC_ARRAY (fg_data, max_pos);
  init_tile_sprites ();

  /* initialize background tile information */

  for (pos = 0; pos < max_pos; ++pos) {

    /* background data */

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

    /* foreground data */

    if (level_map[pos].sprite)
      fg_data[pos].sprite = get_tile_sprite (level_map[pos].sprite);
    else
      fg_data[pos].sprite = 0;
    fg_data[pos].bonus = 0;
    fg_data[pos].big_dollar = 0;
  }
}
