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
#include "camera.h"
#include "const.h"
#include "prefs.h"

static int camera_x[2], camera_y[2]; /* Position of the cameras. */
static int inert_x[2], inert_y[2]; /* Old positions of the cameras,
				      used for intertia.  */
unsigned int corner_x[2];
unsigned int corner_y[2];
pixel_t *(corner[2]);

unsigned int corner_dx[2];
unsigned int corner_dy[2];

unsigned int nbr_tiles_cols = 15; /* # of tiles columns to display */
unsigned int nbr_tiles_rows = 11; /*        ... rows ... */

void
compute_corner (int p, int n)
{
  s32_t x, y;
  s32_t d1, d2, d3;

  s32_t tw = (s32_t)(lvl.tile_width << 16);
  s32_t th = (s32_t)(lvl.tile_height << 16);

  if (opt.inertia) {
    /* when the framerate is too low, don't do inerta */
    if (n > 16)
      n = 16;

    if (lvl.tile_width_wrap == DONT_WRAP)
      inert_x[p] = camera_x[p] =
	inert_x[p] + n * (camera_x[p] - inert_x[p]) / 16;
    else {
      d1 = camera_x[p] - inert_x[p];
      d3 = abs (d1);
      d2 = tw - d3;
      if (d3 <= d2)
	inert_x[p] = camera_x[p] = inert_x[p] + n * (d1) / 16;
      else if (d1 <= 0)
	inert_x[p] = camera_x[p] =
	  inert_x[p] + n * d2 / 16 - tw;
      else
	inert_x[p] = camera_x[p] =
	  inert_x[p] - n * d2 / 16 + tw;
    }
    if (lvl.tile_height_wrap == DONT_WRAP)
      inert_y[p] = camera_y[p] =
	inert_y[p] + n * (camera_y[p] - inert_y[p]) / 16;
    else {
      d1 = camera_y[p] - inert_y[p];
      d3 = abs (d1);
      d2 = th - d3;
      if (d3 <= d2)
	inert_y[p] = camera_y[p] = inert_y[p] + n * (d1) / 16;
      else if (d1 <= 0)
	inert_y[p] = camera_y[p] =
	  inert_y[p] + n * d2 / 16 - th;
      else
	inert_y[p] = camera_y[p] =
	  inert_y[p] - n * d2 / 16 + th;
    }
  }

  camera_x[p] += 81920 * 2 / 3;
  camera_y[p] += 49152;
  x = (camera_x[p] - (nbr_tiles_cols << 15));
  y = (camera_y[p] - (nbr_tiles_rows << 15));
  if (lvl.tile_width_wrap == DONT_WRAP) {
    if (x < 0) {
      x = 0;
    } else if (x > tw - camera_center_x) {
      x = tw - camera_center_x;
    }
  }
  if (lvl.tile_height_wrap == DONT_WRAP) {
    if (y < 0) {
      y = 0;
    } else if (y > th - 655360) {
      y = th - 655360;
    }
  }
  corner_dx[p] = (x >> 16) & lvl.tile_width_wrap;
  corner_dy[p] = (y >> 16) & lvl.tile_height_wrap;
  corner_x[p] = ((x & 0xffff) * 24) >> 16;
  corner_y[p] = ((y & 0xffff) * 20) >> 16;
  corner[p] = render_buffer[p] + sbuf + corner_y[p] * xbuf + corner_x[p];
}

void
position_camera (void)
{
  int p;

  if (player[col2plr[0]].spec == t_tunnel && opt.inertia) {
    p = lvl.square_move[player[col2plr[0]].way][player[col2plr[0]].pos];
    camera_x[0] = square_coord[p].x << 15;
    camera_y[0] = square_coord[p].y << 15;
  } else {
    camera_x[0] = player[col2plr[0]].x2 << 15;
    camera_y[0] = player[col2plr[0]].y2 << 15;
  }

  if (player[col2plr[0]].way == w_left)
    camera_x[0] -= player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_up)
    camera_y[0] -= player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_right)
    camera_x[0] += player[col2plr[0]].d.e >> 1;
  if (player[col2plr[0]].way == w_down)
    camera_y[0] += player[col2plr[0]].d.e >> 1;
  if (two_players) {
    if (player[col2plr[1]].spec == t_tunnel) {
      p = lvl.square_move[player[col2plr[1]].way][player[col2plr[1]].pos];
      camera_x[1] = square_coord[p].x << 15;
      camera_y[1] = square_coord[p].y << 15;
    } else {
      camera_x[1] = player[col2plr[1]].x2 << 15;
      camera_y[1] = player[col2plr[1]].y2 << 15;
    }
    if (player[col2plr[1]].way == w_left)
      camera_x[1] -= player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_up)
      camera_y[1] -= player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_right)
      camera_x[1] += player[col2plr[1]].d.e >> 1;
    if (player[col2plr[1]].way == w_down)
      camera_y[1] += player[col2plr[1]].d.e >> 1;
  }
}

void
init_camera (void)
{
  inert_x[0] = camera_x[0] = player[col2plr[0]].x2 << 15;
  inert_y[0] = camera_y[0] = player[col2plr[0]].y2 << 15;
  inert_x[1] = camera_x[1] = player[col2plr[1]].x2 << 15;
  inert_y[1] = camera_y[1] = player[col2plr[1]].y2 << 15;
}

void
lisajou_camera (int pos)
{
  camera_x[0] = 65536 * 24 * cos (pos / 111.0);
  camera_y[0] = 65536 * 24 * sin (pos / 175.0);
  camera_x[0] &= (lvl.tile_width_wrap << 16) | (0xffff);
  camera_y[0] &= (lvl.tile_height_wrap << 16) | (0xffff);
  inert_x[0] = camera_x[0];
  inert_y[0] = camera_y[0];
}
