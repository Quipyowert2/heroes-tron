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
#include "savemac.h"

/* The body is a succession of tile_width*tile_height records.
   One for each tile.

   Structure of a record:

   Offset  Size  Repeat  What
   --------------------------------------------------------------------
    0      4             sprite offset of the tile in the tile sprite map
    4      1     4       walls for each sub square: if D_UP is set, you
                         cannot *enter* the square from the *bottom* edge.
                         Note this is not the expected content of
                         square_walls_out.  Actually if these bytes
                         set the bit D_LEFT for the square (Y,X), then
			   the same bit should be set in square_walls_out
			   for the square (Y,X+1) (i.e. the square on the
			   right).
    8      2             sprite offset of the overlay in the tile sprite map,
                         unless nul
   10      5             parameters (see interpretation below)
   15      1             tile type
   ==
   16 bytes.
*/
#define TILE_WALLS(p) ((a_u8 *) ((p) + 4))
#define TILE_SPRITE(p) (*(a_u32 *) ((p) + 0))
#define TILE_OVERLAY(p) (*(a_u16 *) ((p) + 8))
#define TILE_TYPE(p) (*(a_tile_type *) ((p) + 15))
/* Parameters are used differently for each type of tile.

   Tunnels:

   Offset  Size  What
   ------------------------
    0      4     output tile index
    4      1     direction (4 lower bits), delay (4 upper bits)
                 delay was never used with tunnels.  The original
                 purpose was to delay the player underground (for the time
                 given by delay, computed by the level editor from the
                 length of the tunnel).  Its has been abandoned because
                 handling of vehicles "out of the map" would complexify
                 the game internals.
   ==
    5 bytes.
*/
#define TUNNEL_OUTPUT(p) (*(a_tile_index *) ((p) + 10))
#define TUNNEL_DIR(p) (*(a_dir8 *) ((p) + 14))
/* Animations:

   Offset  Size  What
   ------------------------
    0      1     frame count
    1      1     delay between each frame (in 70th of sec.)
                 0 is 1/70s, 1 is 2/70s, etc.
    2      3     unused
   ==
    5 bytes.
*/
#define ANIM_FRAME_COUNT(p) (*(a_u8 *) ((p) + 10))
#define ANIM_FRAME_DELAY(p) (*(a_u8 *) ((p) + 11))

/* Stops, Booms, Ices, Dusts, Outway:

   Offset  Size  Repeat  What
   ---------------------------------
    0      1     4       For each square, zero means no effect, non-zero
                         means effect (stop, boom, ice, dust).
    4      1     1       animation: frame count (upper 4 bits), and
                         delay (lower 4 bits, where 0 means 1 and 1 is 16).
   ==
    5 bytes.
*/
#define EFFECT_SET(p) ((a_u8 *) ((p) + 10))
#define SANIM_FRAME_COUNT_DELAY(p) (*(a_u8 *) ((p) + 14))

/* Speeds:

   Offset  Size  Repeat  What
   ---------------------------------
    0      1     4       For each square, the direction*s* of the effect.
    4      1     1       animation: frame count (upper 4 bits), and
                         delay (lower 4 bits, where 0 means 1 and 1 is 16).
   ==
    5 bytes.
*/
#define SPEED_DIR(p) ((a_dir_mask8 *) ((p) + 10))

/* Reverse the walls: Check all neighbour squares if it's allowed to enter
   the current square from there. */
static void
en_reverse_walls (const a_level *lvl, a_dir_mask8 *square_walls_in)
{
  a_square_index idx;

  for (idx = 0; idx < lvl->square_count; ++idx) {
    a_square_index dest;
    a_square_coord this_x, this_y, dest_x, dest_y;

    this_x = SQR_INDEX_TO_COORD_X (lvl, idx);
    this_y = SQR_INDEX_TO_COORD_Y (lvl, idx);

    dest_x = this_x;
    dest_y = SQR_COORD_UP (lvl, this_y);
    dest = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
    if ((this_y == 0) && (lvl->square_height_wrap == DONT_WRAP))
      square_walls_in[idx] |= DM_DOWN;
    else if (lvl->square_walls_out[dest] & DM_DOWN)
      square_walls_in[idx] |= DM_DOWN;
    else if ((lvl->square_type[dest] == T_TUNNEL) &&
	     (lvl->square_direction[dest] == D_DOWN))
      square_walls_in[idx] |= DM_DOWN;

    dest_x = SQR_COORD_RIGHT (lvl, this_x);
    dest_y = this_y;
    dest = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
    if (dest_x >= lvl->square_width)
      square_walls_in[idx] |= DM_LEFT;
    else if (lvl->square_walls_out[dest] & DM_LEFT)
      square_walls_in[idx] |= DM_LEFT;
    else if ((lvl->square_type[dest] == T_TUNNEL) &&
             (lvl->square_direction[dest] == D_LEFT))
      square_walls_in[idx] |= DM_LEFT;

    dest_x = this_x;
    dest_y = SQR_COORD_DOWN (lvl, this_y);
    dest = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
    if (dest_y >= lvl->square_height)
      square_walls_in[idx] |= DM_UP;
    else if (lvl->square_walls_out[dest] & DM_UP)
      square_walls_in[idx] |= DM_UP;
    else if ((lvl->square_type[dest] == T_TUNNEL) &&
             (lvl->square_direction[dest] == D_UP))
      square_walls_in[idx] |= DM_UP;

    dest_x = SQR_COORD_LEFT (lvl, this_x);
    dest_y = this_y;
    dest = SQR_COORDS_TO_INDEX (lvl, dest_y, dest_x);
    if ((this_x == 0) && (lvl->square_width_wrap == DONT_WRAP))
      square_walls_in[idx] |= DM_RIGHT;
    else if (lvl->square_walls_out[dest] & DM_RIGHT)
      square_walls_in[idx] |= DM_RIGHT;
    else if ((lvl->square_type[dest] == T_TUNNEL) &&
             (lvl->square_direction[dest] == D_RIGHT))
      square_walls_in[idx] |= DM_RIGHT;
  }
}

/* Encode the level body into the preallocated buffer data */
void
encode_level_body (a_u8 *data, const a_level *lvl)
{
  a_tile_index ti;		/* Current tile index. */
  a_square_index si;		/* Current square index. */
  a_tile_index tcount;		/* Total tile count to write.  */
  a_dir_mask8 *square_walls_in;	/* Walls forbiding to *enter* a tile.  */

  tcount = lvl->tile_count;
  square_walls_in = xmalloc (lvl->square_count);
  /* T_OUTWAY squares cannot be entered from any direction */
  for (si = 0; si < lvl->square_count; ++si)
    if (lvl->square_type[si] == T_OUTWAY)
      square_walls_in[si] = DM_ALL;
  en_reverse_walls (lvl, square_walls_in);

  /* Write each tile to buffer. */
  for (ti = 0; ti < tcount; ++ti, data += LVL_RECORD_SIZE) {
    a_tile_type tt;		/* Tile type.  */

    si = TILE_INDEX_TO_SQR_INDEX (lvl, ti);

    /* Store tile type.  */
    tt = lvl->private->tile[ti].type;
    TILE_TYPE (data) = tt;

    /* Store inside walls. */
    TILE_WALLS (data)[0] = square_walls_in[SQR0 (lvl, si)];
    TILE_WALLS (data)[1] = square_walls_in[SQR1 (lvl, si)];
    TILE_WALLS (data)[2] = square_walls_in[SQR2 (lvl, si)];
    TILE_WALLS (data)[3] = square_walls_in[SQR3 (lvl, si)];

    /* Store sprites.  */
    TILE_SPRITE (data) = lvl->private->tile[ti].sprite_offset;
    TILE_OVERLAY (data) = lvl->private->tile[ti].sprite_overlay_offset;

    switch (tt) {
    case T_TUNNEL:
      {
	int s;
	a_dir td;
	a_square_index tsi;
	a_tile_index dti;

	/* find a square that's part of the tunnel */
	s = 0;
	while (lvl->square_type[SQRX (lvl, si, s)] != T_TUNNEL) ++s;
	tsi = SQRX (lvl, si, s);

	td = lvl->square_direction[tsi];
	dti = SQR_INDEX_TO_TILE_INDEX (lvl, lvl->square_move[td][tsi]);
	TUNNEL_OUTPUT (data) = dti;
	TUNNEL_DIR (data) = DIR_TO_DIRMASK (td);
      }
      break;
    case T_ANIM:
      ANIM_FRAME_COUNT (data) = lvl->private->tile[ti].frame_count;
      ANIM_FRAME_DELAY (data) = lvl->private->tile[ti].frame_delay - 1;
      break;
    case T_SPEED:
      {
	int x;
	for (x = 0; x < 4; ++x)
	  if (lvl->square_type[SQRX (lvl, si, x)] != T_NONE)
	    SPEED_DIR (data)[x] |=
	      DIR_TO_DIRMASK (lvl->square_direction[SQRX (lvl, si, x)]);
      }
      goto decode_small_anim;
    case T_STOP:
    case T_BOOM:
    case T_ICE:
    case T_DUST:
      {
	int x;
	/* Set EFFECT_SET=non_zero for all squares that have
	   some special effect enabled  */
	for (x = 0; x < 4; ++x)
	  if (lvl->square_type [SQRX (lvl, si, x)] == T_NONE)
	    EFFECT_SET (data)[x] = 0;
	  else
	    EFFECT_SET (data)[x] = 1;
      }
      /* Fall through.  */
    decode_small_anim:
    case T_OUTWAY:
    case T_NONE:
      SANIM_FRAME_COUNT_DELAY (data) =
	(lvl->private->tile[ti].frame_count << 4) |
	(lvl->private->tile[ti].frame_delay - 1);
      break;
    default:
      assert (0);
    }
  }
  free (square_walls_in);
}
