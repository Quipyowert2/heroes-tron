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

static int
tileset_load_mem (const a_u8 *data, a_tileset *tset)
{
  a_tile_index ti;

  for (ti = 0; ti < tset->tile_count; ++ti, data += PIE_RECORD_SIZE) {
    int subsquare;
    a_square_index si = TILE_INDEX_TO_SQR_INDEX (tset, ti);
    a_tile_type tt = PIE_GET_TILE_TYPE (data);

    tset->tile[ti].type = tt;

    for (subsquare = 0; subsquare < 4; ++subsquare) {
      int ssi = SQRX (tset, si, subsquare);
      tset->square[ssi].type = tt;
      tset->square[ssi].walls_in = PIE_GET_TILE_WALLS (data)[subsquare];
    }

    tset->tile[ti].sprite_offset =
	(ti/tset->tile_width) * tset->image_width * PIE_TILE_PIXELS_Y +
	(ti%tset->tile_width) * PIE_TILE_PIXELS_X;

    switch (tt) {
    case T_TUNNEL:
      {
	a_dir td = PIE_GET_TUNNEL_DIR (data);
	tset->square[SQRX (tset, si, tunnel_square_io[td][0])].direction = td;
	tset->square[SQRX (tset, si, tunnel_square_io[td][1])].direction = td;
	tset->square[SQRX (tset, si, tunnel_square_io[td^2][0])].type = T_NONE;
	tset->square[SQRX (tset, si, tunnel_square_io[td^2][1])].type = T_NONE;
      }
      break;
    case T_ANIM:
      tset->tile[ti].frame_count = PIE_GET_ANIM_FRAME_COUNT (data);
      tset->tile[ti].frame_delay = PIE_GET_ANIM_FRAME_DELAY (data);
      tset->tile[ti].anim = A_LOOP;
      break;
    case T_SPEED:
      {
        int x;
        for (x = 0; x < 4; ++x) {
          a_dir_mask dm = PIE_GET_SPEED_DIR (data)[x];
          if (dm)
            tset->square[SQRX (tset, si, x)].direction = dir_mask_to_dir (dm);
          else
            tset->square[SQRX (tset, si, x)].type = T_NONE;
        }
      }
      goto decode_small_anim;
    case T_STOP:
    case T_BOOM:
    case T_ICE:
    case T_DUST:
      {
        int x;
        /* The type of each square is already set, but we don't want
           it on the squares where effects are not enabled.  */
        for (x = 0; x < 4; ++x)
          if (! PIE_GET_EFFECT_SET (data)[x])
            tset->square[SQRX (tset, si, x)].type = T_NONE;
      }
      /* Fall through.  */
    decode_small_anim:
    case T_OUTWAY:
    case T_NONE:
      tset->tile[ti].frame_count = PIE_GET_SANIM_FRAME_COUNT (data);
      tset->tile[ti].frame_delay = PIE_GET_SANIM_FRAME_DELAY (data);
      if (tset->tile[ti].frame_count)
        tset->tile[ti].anim = A_PINGPONG;
      else
        tset->tile[ti].anim = A_NONE;
      break;
    default:
      assert (0);
    }
  }
  return 1;
}

int
lvl_load_tileset (a_level *lvl)
{
  int fd;
  int err;
  char* pie_file;
  a_tileset *tset;
#ifdef HAVE_MMAP
  bool use_mmap = false;
#endif
  a_u8 *data;
  struct stat st;

  /* allocate and initialize the a_tileset structure in *level */
  XFREE0 (lvl->tileset);
  tset = XMALLOC_VAR (lvl->tileset);

  /* FIXME: read image dimensions from the pcx file */
  tset->image_width = PIE_PCX_PIXELS_X;
  tset->image_height = PIE_PCX_PIXELS_Y;
  tset->tile_width = tset->image_width / PIE_TILE_PIXELS_X;
  tset->tile_height = tset->image_height / PIE_TILE_PIXELS_Y;
  tset->square_width = 2 * tset->tile_width;
  tset->square_height = 2 * tset->tile_height;

  tset->tile_count = tset->tile_width * tset->tile_height;
  tset->square_count = tset->square_width * tset->square_height;
  XSALLOC_ARRAY (tset->tile, tset->tile_count, 0);
  XSALLOC_ARRAY (tset->square, tset->square_count, 0);

  /* assemble the ".pie" file name */
  pie_file = xmalloc (strlen (PREFIX) + 1 +
      strlen (FORWARD_RELATIVE_PKGDATADIR) + 10 +
      strlen (lvl->private->tile_sprite_map_basename) + 5);
  strcpy (pie_file, PREFIX "/" FORWARD_RELATIVE_PKGDATADIR "/tilesets/");
  strcat (pie_file, lvl->private->tile_sprite_map_basename);
  strcat (pie_file, ".pie");
  fd = open (pie_file, O_RDONLY | O_BINARY);
  free (pie_file);
  if (fd == -1)
    return -1;

#ifdef HAVE_MMAP
  /* We need the size of the file in order to mmap() it.  */
  if (fstat (fd, &st) != 0) {
    close (fd);
    return -1;
  }

  /* Try to mmap the file.  If it fail for any reason, we'll
     fall back to the standard file reading method.  */
  data = mmap (0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data != (void*) -1) {
    /* Success.  */
    use_mmap = true;
    close (fd);
  }

  if (!use_mmap)
#endif
  {
    size_t length = tset->tile_count * PIE_RECORD_SIZE;
    ssize_t rlength;
    data = xmalloc (length);
    rlength = read (fd, data, length);

    if (rlength < 0 || (size_t) rlength != length) {
      free (data);
      err = -1;
    } else
      err = tileset_load_mem (data, tset);

    close (fd);
  }
#ifdef HAVE_MMAP
  else {
    err = tileset_load_mem (data, tset);
    munmap (data, st.st_size);
  }
#endif

  /* reset all walls to the defaults given in the tileset. This is
     necessary before map-editing to get rid of walls that are raised
     only to mark T_OUTWAY squares in level files */
  if (err == 1) {
    a_tile_index tile;
    a_square_index square;
    a_square_coord x, y;

    for (square = 0; square < lvl->square_count; ++square) {
      x = SQR_INDEX_TO_COORD_X (lvl, square);
      y = SQR_INDEX_TO_COORD_Y (lvl, square);
      lvl->square_walls_out[square] = 0;
      lvl->square_move[D_UP][square] =
        SQR_COORDS_TO_INDEX (lvl, SQR_COORD_UP (lvl, y), x);
      lvl->square_move[D_RIGHT][square] =
        SQR_COORDS_TO_INDEX (lvl, y, SQR_COORD_RIGHT (lvl, x));
      lvl->square_move[D_DOWN][square] =
        SQR_COORDS_TO_INDEX (lvl, SQR_COORD_DOWN (lvl, y), x);
      lvl->square_move[D_LEFT][square] =
        SQR_COORDS_TO_INDEX (lvl, y, SQR_COORD_LEFT (lvl, x));
    }

    if (lvl->square_width_wrap == DONT_WRAP)
      for (y = 0; y < lvl->square_height; ++y) {
	lvl->square_move[D_LEFT][SQR_COORDS_TO_INDEX (lvl, y, 0)]
	  = INVALID_INDEX;
	lvl->square_move[D_RIGHT][SQR_COORDS_TO_INDEX (lvl, y,
						lvl->square_width - 1)]
	  = INVALID_INDEX;
      }

    if (lvl->square_height_wrap == DONT_WRAP)
      for (x = 0; x < lvl->square_width; ++x) {
	lvl->square_move[D_UP][SQR_COORDS_TO_INDEX (lvl, 0, x)]
	  = INVALID_INDEX;
	lvl->square_move[D_DOWN][SQR_COORDS_TO_INDEX (lvl,
						lvl->square_height - 1, x)]
	  = INVALID_INDEX;
      }

    for (tile = 0; tile < lvl->tile_count; ++tile)
      rebuild_walls (lvl, tile);
  }

  return err;
}
