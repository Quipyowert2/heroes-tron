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
#include "lvl_priv.h"
#include "format.h"
#include "xstrduplwr.h"

int lvl_create (a_level *lvl, a_tile_coord height, a_tile_coord width,
	       bool wrap_y, bool wrap_x, const char* tile_sprite_map_basename)
{
  /* Sanity checks */
  if (wrap_y && (height != 16) && (height != 32) && (height != 64))
    return 0;
  if (wrap_x && (width != 16) && (width != 32) && (width != 64))
    return 0;
  if ((height < 11) || (height > 64) || (width < 15) || (width > 64))
    return 0;

  /* Initialize the basic fields */
  memset (lvl, 0, sizeof *lvl);
  XMALLOC_VAR (lvl->private);
  lvl->tileset = 0;

  lvl->tile_width = width;
  lvl->tile_height = height;
  lvl->square_width = 2 * width;
  lvl->square_height = 2 * height;
  lvl->tile_count = width * height;
  lvl->square_count = 4 * width * height;

  if (wrap_y) {
    lvl->tile_height_wrap = lvl->tile_height - 1;
    lvl->square_height_wrap = lvl->square_height - 1;
  } else {
    lvl->tile_height_wrap = DONT_WRAP;
    lvl->square_height_wrap = DONT_WRAP;
  }

  if (wrap_x) {
    lvl->tile_width_wrap = lvl->tile_width - 1;
    lvl->square_width_wrap = lvl->square_width - 1;
  } else {
    lvl->tile_width_wrap = DONT_WRAP;
    lvl->square_width_wrap = DONT_WRAP;
  }

  initialize_level_body (lvl);

  lvl->private->tile_sprite_map_basename =
		xstrduplwr (tile_sprite_map_basename);
  lvl->private->sound_track_alias = 0;

  /* set default start positions and directions */
  lvl->private->start_pos[0].x = 0;
  lvl->private->start_pos[0].y = 0;
  lvl->private->start_pos[1].x = 1;
  lvl->private->start_pos[1].y = 0;
  lvl->private->start_pos[2].x = 0;
  lvl->private->start_pos[2].y = 1;
  lvl->private->start_pos[3].x = 1;
  lvl->private->start_pos[3].y = 1;

  lvl->private->start_dir[0] = D_UP;
  lvl->private->start_dir[1] = D_RIGHT;
  lvl->private->start_dir[2] = D_LEFT;
  lvl->private->start_dir[3] = D_DOWN;

  return 1;
}

static a_tile_index
sprite_offset_to_tileset_index (a_tileset *tset, int spr_ofs)
{
  int spr_ofs_y = spr_ofs / tset->image_width;
  int spr_ofs_x = spr_ofs % tset->image_width;
  a_tile_index tset_idx = (spr_ofs_y / PIE_TILE_PIXELS_Y) * tset->tile_width +
			  (spr_ofs_x / PIE_TILE_PIXELS_X);
  return tset_idx;
}

/* mark all tiles that cannot be entered by any player as T_OUTWAY */
static void
mark_outway (a_level *lvl)
{
  a_u8 *outway_map;  /* 0: not yet visited
			1: recently visited (during last move)
			2: visited earlier */
  bool finished = false; /* no more moves, game over */
  a_square_index sidx;
  a_tile_index tidx;
  int i;
  XCALLOC_ARRAY (outway_map, lvl->square_count);

  /* reset all T_OUTWAYs to their original type */
  for (tidx = 0; tidx < lvl->tile_count; ++tidx)
    if (lvl->private->tile[tidx].type == T_OUTWAY) {
      a_tile_index tset_idx = sprite_offset_to_tileset_index
	(lvl->tileset, lvl->private->tile[tidx].sprite_offset);
      a_square_index tset_square =
	TILE_INDEX_TO_SQR_INDEX (lvl->tileset, tset_idx);
      sidx = TILE_INDEX_TO_SQR_INDEX (lvl, tidx);
      lvl->private->tile[tidx].type = lvl->tileset->tile[tset_idx].type;
      for (i = 0; i < 4; ++i)
	lvl->square_type[SQRX (lvl, sidx, i)] =
	  lvl->tileset->square[SQRX (lvl->tileset, tset_square, i)].type;
    }

  /* The players' starting positions are the origins of the accessibility
     check algorithm. Mark them as "recently visited" */
  for (i = 0; i < 4; ++i) {
    a_square_index square = SQR_COORDS_TO_INDEX (lvl,
	lvl->private->start_pos[i].y, lvl->private->start_pos[i].x);
    outway_map[square] = 1;
  }

  /* find unexplored squares next to recently visited ones. If no more
     accessible squares are found, stop iterating */
  while (!finished) {
    finished = true; /* assume there are no more squares to explore */
    for (sidx = 0; sidx < lvl->square_count; ++sidx)
      if (outway_map[sidx] == 1) {
	outway_map[sidx] = 2;
	for (i = 0; i < 4; ++i) { /* explore each neighbour */
	  a_square_index dest = lvl->square_move[i][sidx];
	  if ((dest != INVALID_INDEX) && (outway_map[dest] == 0)) {
	    finished = false;
	    outway_map[dest] = 1;
	  }
	}
      }
  }

  /* mark unvisited tiles/squares as T_OUTWAY */
  for (tidx = 0; tidx < lvl->tile_count; ++tidx) {
    sidx = TILE_INDEX_TO_SQR_INDEX (lvl, tidx);
    if (((outway_map[SQR0 (lvl, sidx)] == 0) ||
	 (outway_map[SQR1 (lvl, sidx)] == 0) ||
	 (outway_map[SQR2 (lvl, sidx)] == 0) ||
	 (outway_map[SQR3 (lvl, sidx)] == 0)) &&
	(lvl->private->tile[tidx].type != T_TUNNEL)) {
      lvl->private->tile[tidx].type = T_OUTWAY;
      for (i = 0; i < 4; ++i) {
	a_square_index dest = SQRX (lvl, sidx, i);
	if (outway_map[dest] == 0)
	  lvl->square_type[dest] = T_OUTWAY;
	else
	  lvl->square_type[dest] = T_NONE;
      }
    }
  }

  XFREE (outway_map);
}

static void
rebuild_walls (a_level *lvl, a_tile_index idx)
{
  a_tileset *tset = lvl->tileset;
  a_square_index d_square0 = TILE_INDEX_TO_SQR_INDEX (lvl, idx);
  a_square_index s_square0;
  int subsquare;

  a_tile_index tset_idx = sprite_offset_to_tileset_index
		(tset, lvl->private->tile[idx].sprite_offset);
  s_square0 = TILE_INDEX_TO_SQR_INDEX (tset, tset_idx);

  /* Add the walls given in tileset square information */
  for (subsquare = 0; subsquare < 4; ++subsquare) {
    a_square_index s_square = SQRX (tset, s_square0, subsquare);
    /* index and coordinates of current subsquare */
    a_square_index square_i = SQRX (lvl, d_square0, subsquare);
    a_square_coord square_x = SQR_INDEX_TO_COORD_X (lvl, square_i);
    a_square_coord square_y = SQR_INDEX_TO_COORD_Y (lvl, square_i);

    /* add this square's walls and propagate them to its neighbours */
    { /* up */
      a_square_coord dest_x = square_x;
      a_square_coord dest_y = SQR_COORD_UP (lvl, square_y);
      if (SQR_COORD_Y_VALID (lvl, dest_y)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	if (tset->square[s_square].walls_in & DM_DOWN) {
	  lvl->square_walls_out[square_i] |= DM_UP;
	  lvl->square_move[D_UP][square_i] = INVALID_INDEX;
	  lvl->square_walls_out[dest_i] |= DM_DOWN;
	  lvl->square_move[D_DOWN][dest_i] = INVALID_INDEX;
	}
      }
    }

    { /* right */
      a_square_coord dest_x = SQR_COORD_RIGHT (lvl, square_x);
      a_square_coord dest_y = square_y;
      if (SQR_COORD_X_VALID (lvl, dest_x)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	if (tset->square[s_square].walls_in & DM_LEFT) {
	  lvl->square_walls_out[square_i] |= DM_RIGHT;
	  lvl->square_move[D_RIGHT][square_i] = INVALID_INDEX;
	  lvl->square_walls_out[dest_i] |= DM_LEFT;
	  lvl->square_move[D_LEFT][dest_i] = INVALID_INDEX;
	}
      }
    }

    { /* down */
      a_square_coord dest_x = square_x;
      a_square_coord dest_y = SQR_COORD_DOWN (lvl, square_y);
      if (SQR_COORD_Y_VALID (lvl, dest_y)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	if (tset->square[s_square].walls_in & DM_UP) {
	  lvl->square_walls_out[square_i] |= DM_DOWN;
	  lvl->square_move[D_DOWN][square_i] = INVALID_INDEX;
	  lvl->square_walls_out[dest_i] |= DM_UP;
	  lvl->square_move[D_UP][dest_i] = INVALID_INDEX;
	}
      }
    }

    { /* left */
      a_square_coord dest_x = SQR_COORD_LEFT (lvl, square_x);
      a_square_coord dest_y = square_y;
      if (SQR_COORD_X_VALID (lvl, dest_x)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	if (tset->square[s_square].walls_in & DM_RIGHT) {
	  lvl->square_walls_out[square_i] |= DM_LEFT;
	  lvl->square_move[D_LEFT][square_i] = INVALID_INDEX;
	  lvl->square_walls_out[dest_i] |= DM_RIGHT;
	  lvl->square_move[D_RIGHT][dest_i] = INVALID_INDEX;
	}
      }
    }
  }
}

int lvl_assign_tile (a_level *lvl, a_tile_index dest, a_tile_index src)
{
  a_square_index d_square0, s_square0;
  int subsquare;

  a_tileset *tset = lvl->tileset;
  if (! tset)
    return 0; /* tileset has to be loaded */

  d_square0 = TILE_INDEX_TO_SQR_INDEX (lvl, dest);
  s_square0 = TILE_INDEX_TO_SQR_INDEX (tset, src);

  /* copy tile properties */
  lvl->private->tile[dest].type = tset->tile[src].type;
  lvl->private->tile[dest].sprite_offset = tset->tile[src].sprite_offset;
  lvl->private->tile[dest].sprite_overlay_offset = 0;
  lvl->private->tile[dest].frame_count = tset->tile[src].frame_count;
  lvl->private->tile[dest].frame_delay = tset->tile[src].frame_delay;
  lvl->private->tile[dest].anim = tset->tile[src].anim;

  /* copy square properties */
  for (subsquare = 0; subsquare < 4; ++subsquare) {
    a_square_index s_square = SQRX (tset, s_square0, subsquare);
    /* index and coordinates of current subsquare */
    a_square_index square_i = SQRX (lvl, d_square0, subsquare);
    a_square_coord square_x = SQR_INDEX_TO_COORD_X (lvl, square_i);
    a_square_coord square_y = SQR_INDEX_TO_COORD_Y (lvl, square_i);

    /* copy type and direction */
    lvl->square_type[square_i] = tset->square[s_square].type;
    lvl->square_direction[square_i] = tset->square[s_square].direction;

    /* clear all walls (gonna rebuild them later) */
    { /* up */
      a_square_coord dest_x = square_x;
      a_square_coord dest_y = SQR_COORD_UP (lvl, square_y);
      if (SQR_COORD_Y_VALID (lvl, dest_y)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	lvl->square_walls_out[square_i] &= ~DM_UP;
	lvl->square_move[D_UP][square_i] = dest_i;
	lvl->square_walls_out[dest_i] &= ~DM_DOWN;
	lvl->square_move[D_DOWN][dest_i] = square_i;
      }
    }

    { /* right */
      a_square_coord dest_x = SQR_COORD_RIGHT (lvl, square_x);
      a_square_coord dest_y = square_y;
      if (SQR_COORD_X_VALID (lvl, dest_x)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	lvl->square_walls_out[square_i] &= ~DM_RIGHT;
	lvl->square_move[D_RIGHT][square_i] = dest_i;
	lvl->square_walls_out[dest_i] &= ~DM_LEFT;
	lvl->square_move[D_LEFT][dest_i] = square_i;
      }
    }

    { /* down */
      a_square_coord dest_x = square_x;
      a_square_coord dest_y = SQR_COORD_DOWN (lvl, square_y);
      if (SQR_COORD_Y_VALID (lvl, dest_y)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	lvl->square_walls_out[square_i] &= ~DM_DOWN;
	lvl->square_move[D_DOWN][square_i] = dest_i;
	lvl->square_walls_out[dest_i] &= ~DM_UP;
	lvl->square_move[D_UP][dest_i] = square_i;
      }
    }

   { /* left */
      a_square_coord dest_x = SQR_COORD_LEFT (lvl, square_x);
      a_square_coord dest_y = square_y;
      if (SQR_COORD_X_VALID (lvl, dest_x)) {
	a_square_index dest_i = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
	lvl->square_walls_out[square_i] &= ~DM_LEFT;
	lvl->square_move[D_LEFT][square_i] = dest_i;
	lvl->square_walls_out[dest_i] &= ~DM_RIGHT;
	lvl->square_move[D_RIGHT][dest_i] = square_i;
      }
    }
  }

  /* rebuild the walls for this tile and all neighbour tiles */
  rebuild_walls (lvl, dest);

  { /* up */
    a_tile_coord dtile_x = TILE_INDEX_TO_COORD_X (lvl, dest);
    a_tile_coord dtile_y = (TILE_INDEX_TO_COORD_Y (lvl, dest) - 1)
			    & lvl->tile_height_wrap;
    if (dtile_y < lvl->tile_height)
      rebuild_walls (lvl, TILE_COORDS_TO_INDEX (lvl, dtile_y, dtile_x));
  }

  { /* right */
    a_tile_coord dtile_x = (TILE_INDEX_TO_COORD_X (lvl, dest) + 1)
			    & lvl->tile_width_wrap;
    a_tile_coord dtile_y = TILE_INDEX_TO_COORD_Y (lvl, dest);
    if (dtile_x < lvl->tile_width)
      rebuild_walls (lvl, TILE_COORDS_TO_INDEX (lvl, dtile_y, dtile_x));
  }

  { /* down */
    a_tile_coord dtile_x = TILE_INDEX_TO_COORD_X (lvl, dest);
    a_tile_coord dtile_y = (TILE_INDEX_TO_COORD_Y (lvl, dest) + 1)
			    & lvl->tile_height_wrap;
    if (dtile_y < lvl->tile_height)
      rebuild_walls (lvl, TILE_COORDS_TO_INDEX (lvl, dtile_y, dtile_x));
  }

  { /* left */
    a_tile_coord dtile_x = (TILE_INDEX_TO_COORD_X (lvl, dest) - 1)
			    & lvl->tile_width_wrap;
    a_tile_coord dtile_y = TILE_INDEX_TO_COORD_Y (lvl, dest);
    if (dtile_x < lvl->tile_width)
      rebuild_walls (lvl, TILE_COORDS_TO_INDEX (lvl, dtile_y, dtile_x));
  }

  mark_outway (lvl);

  return 1;
}

void lvl_set_soundtrack (a_level *lvl, const char *sound_track_alias)
{
  XFREE0 (lvl->private->sound_track_alias);
  lvl->private->sound_track_alias = xstrdup (sound_track_alias);
}

int lvl_set_start (a_level *lvl, int player, a_square_index idx, a_dir dir)
{
  if (! lvl->tileset)
    return 0;
  if (idx >= lvl->square_count)
    return 0;
  if ((dir != D_UP) && (dir != D_RIGHT) && (dir != D_DOWN) && (dir != D_LEFT))
    return 0;
  lvl->private->start_pos[player].x = SQR_INDEX_TO_COORD_X (lvl, idx);
  lvl->private->start_pos[player].y = SQR_INDEX_TO_COORD_Y (lvl, idx);
  lvl->private->start_dir[player] = dir;
  mark_outway (lvl);
  return 1;
}

int lvl_setup_tunnel (a_level *lvl, a_square_index start, a_square_index end)
{
  int td = lvl->square_direction[start];
  if (! lvl->tileset)
    return 0;
  if (lvl->square_type[start] != T_TUNNEL)
    return 0;
  if (lvl->square_type[end] != T_TUNNEL)
    return 0;
  lvl->square_move[td][start] = end;
  mark_outway (lvl);
  return 1;
}

void lvl_set_anim_delay (a_level *lvl, a_tile_index idx, unsigned int delay)
{
  lvl->private->tile[idx].frame_delay = delay;
}
