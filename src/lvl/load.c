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

static int
lvl_load_header (int fd, level_t *out)
{
  u8_t *data = xmalloc (LVL_HEADER_SIZE);

  if (read (fd, data, LVL_HEADER_SIZE) != LVL_HEADER_SIZE) {
    free (data);
    return -1;
  }

  decode_level_header (data, out);

  /* Compute the other `trivial' fields.  */

  out->square_height = out->tile_height * 2;
  out->square_width = out->tile_width * 2;

  if (out->tile_height_wrap == DONT_WRAP)
    out->square_height_wrap = DONT_WRAP;
  else
    out->square_height_wrap = (out->tile_height_wrap << 1) | 1;

  if (out->tile_width_wrap == DONT_WRAP)
    out->square_width_wrap = DONT_WRAP;
  else
    out->square_width_wrap = (out->tile_width_wrap << 1) | 1;

  out->tile_count = out->tile_height * out->tile_width;
  out->square_count = out->square_height * out->square_width;

  free (data);
  return 0;
}

static int
lvl_load_body (int fd, level_t *out)
{
  ssize_t length = out->tile_count * LVL_RECORD_SIZE;
  u8_t *data = xmalloc (length);

  if (read (fd, data, length) != length) {
    free (data);
    return -1;
  }

  initialize_level_body (out);
  decode_level_body (data, out);

  free (data);
  return 0;
}

int
lvl_load_file (const char *filename, level_t *out, bool load_body)
{
  int fd = open (filename, O_RDONLY | O_BINARY);
  int err;

  memset (out, 0, sizeof *out);

  if (fd == -1)
    return -1;

  XMALLOC_VAR (out->private);

  err = lvl_load_header (fd, out);

  if (!err && load_body)
    err = lvl_load_body (fd, out);

  close (fd);

  return err;
}
