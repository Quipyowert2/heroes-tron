/*------------------------------------------------------------------.
| Copyright 1997, 1998, 2000, 2001  Alexandre Duret-Lutz            |
|                                    <duret_g@epita.fr>             |
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
#include "display.h"
#include "const.h"
#include "prefs.h"
#include "fastmem.h"
#include "render.h"
#include "argv.h"
#include "timer.h"
#include "heroes.h"
#include "renderdata.h"
#include "bonus.h"
#include "explosions.h"
#include "items.h"
#include "sprglenz.h"
#include "sprunish.h"
#include "menus.h"
#include "camera.h"

char tutor = 0;
static sprite_t *clock_anim;

static bool invincible[4];	/* When a cell is true, the corresponding
				   player is highlighted.  This happens
				   pariodically when the player is
				   invincible (blinking). */


static void
copy_tile (const pixel_t* src, pixel_t* dest, int tx)
{
  const u32_t *s = (const u32_t *) src;
  u32_t *d = (u32_t *) dest;
  u32_t t1, t2;
  int y;
  for (y = 20; y; --y) {	/* FIXME: Is this really faster than a */
    t1 = s[0];			/* straight copy?  Need a benchmark  */
    t2 = s[3];
    d[0] = t1;
    d[3] = t2;
    t1 = s[1];
    t2 = s[4];
    d[1] = t1;
    d[4] = t2;
    t1 = s[2];
    t2 = s[5];
    d[2] = t1;
    d[5] = t2;
    s = (const u32_t *) (((const pixel_t *) s) + tx);
    d = (u32_t *) (((pixel_t *) d) + xbuf);
  }
}

static void
copy_square_transp (const pixel_t* src, pixel_t* dest, char d, char e)
{
  int j, k;
  pixel_t c;
  for (j = 10 - e; j != 0; j--) {
    for (k = 12 - d; k != 0; k--) {
      c = *src++;
      if (c != 0)
	*dest = c;
      dest++;
    }
    src += 320 - 12 + d;
    dest += xbuf - 12 + d;
  }
}

static void
draw_trail_real (int c, unsigned char s, pixel_t* dest,
		 unsigned char fixe)
{
  const sprite_t *spr;
  int d = 0;

  if (fixe) {
    if (s & 1)
      d = player[c].d.h.l / 5462;
    else
      d = player[c].d.h.l / 6554;
  }
  spr = trails[s][d];
  draw_sprglenz_custom (spr, dest, glenz[c + 2]);
}

static void
draw_vehicle_tail (int c, pixel_t* dest)
{
  int d;
  const pixel_t* posit = 0;
  int s = ((player[c].old_way ^ 2) + ((player[c].way ^ 2) << 2));
  const sprite_t *spr;

  if (s & 4) {
    d = player[c].d.h.l / 5462;
    spr = trails[s][12 - (d + 12) / 2];
  } else {
    d = player[c].d.h.l / 6554;
    spr = trails[s][10 - (d + 10) / 2];
  }
  draw_sprglenz_custom (spr, dest, glenz[c + 2]);

  posit = vehicles_img.buffer + (c << 6) + (player[c].way << 4);
  if (invincible[c])
    posit += 10 * 320;
  if (player[c].way == w_left)
    copy_square_transp (posit + d, dest, (char) d, 0);
  if (player[c].way == w_right)
    copy_square_transp (posit, dest + d, (char) d, 0);
  if (player[c].way == w_up)
    copy_square_transp (posit + d * 320, dest, 0, (char) d);
  if (player[c].way == w_down)
    copy_square_transp (posit, dest + d * xbuf, 0, (char) d);
}

static void
draw_vehicle_head (int c, pixel_t* dest)
{
  int d;
  char b;
  const pixel_t* posit;

  if (player[c].spec == t_tunnel)
    b = player[c].tunnel_way;
  else
    b = player[c].way;
  posit = vehicles_img.buffer + (c << 6) + (b << 4);
  if (invincible[c])
    posit += 10 * 320;

  if (b == w_left) {
    d = 12 - player[c].d.e / 5461;
    copy_square_transp (posit, dest + d, (char) d, 0);
  }
  if (b == w_right) {
    d = 12 - player[c].d.e / 5461;
    copy_square_transp (posit + d, dest, (char) d, 0);
  }
  if (b == w_up) {
    d = 10 - player[c].d.e / 6554;
    copy_square_transp (posit, dest + d * xbuf, 0, (char) d);
  }
  if (b == w_down) {
    d = 10 - player[c].d.e / 6554;
    copy_square_transp (posit + d * 320, dest, 0, (char) d);
  }
}

static void
draw_trail (int c, pixel_t* dest, char d)
{
  draw_trail_real ((char) (c - 2), (char) (d & 15), dest, 0);
}

static void
draw_trail_tail (int c, pixel_t* dest)
{
  char k;
  int tmp1;
  c -= 2;
  tmp1 = trail_offset[c] + trail_size[c] - 1;
  k = trail_way[c][tmp1 & (maxq - 1)];
  if (trail_pos[c][tmp1 & (maxq - 1)]
      != trail_pos[c][(tmp1 - 1) & (maxq - 1)])
    draw_trail_real ((char) c, (char) k, dest, 1);
  else
    draw_trail_real ((char) c, (char) k, dest, 0);
}

static void
copy_lemming_transp (const pixel_t* src, pixel_t *dest)
{
  int j, k;

  for (j = 8; j != 0; j--) {
    for (k = 8; k != 0; k--) {
      if (*src != 0) {
	if (*src == 1)
	  *dest = glenz[0][*dest];
	else
	  *dest = *src;
      }
      src++;
      dest++;
    }
    src += 320 - 8;
    dest += xbuf - 8;
  }
}

static void
draw_color (pixel_t* dest, int c)
{
  if (c & 16)
    DRAW_SPRITE (clock_anim, dest - 2 * xbuf);
  else
    DRAW_SPRITE (pyramids[c & 7], dest);

  if (c & 8) {
    int j, k;
    const pixel_t* src = main_font_img.buffer + 81 * 320 + 40;
    dest -= 2 + xbuf;
    for (j = 7; j != 0; j--) {
      for (k = 12; k != 0; k--) {
	if (*src == 16)
	  *dest = glenz[6][*dest];
	src++;
	dest++;
      }
      src += 320 - 12;
      dest += xbuf - 12;
    }
  }
}

static void
draw_lemming (pixel_t* dest, const lemming_t* ptibptr, unsigned int pos)
{
  char c, d;
  const pixel_t *src;

  if (ptibptr >= lemmings_support
      && ptibptr < lemmings_support + lemmings_total && pos == ptibptr->pos1) {
    c = ptibptr->couleur;
    d = ptibptr->way;
    src = vehicles_img.buffer + 164 * 320;
    if (d != 5)
      src += lemmings_anim_offset;
    if (d & 1)
      src += 8 * 320;
    src += 64 * c;
    if (d == w_up)
      dest -= (lemmings_move_offset / 6553) * xbuf;
    if (d == w_right)
      dest += lemmings_move_offset / 5461;
    if (d == w_down)
      dest += (lemmings_move_offset / 6553) * xbuf;
    if (d == w_left)
      dest -= lemmings_move_offset / 5461;
    copy_lemming_transp (src, dest);
  }

}

static void
copy_dead_lemming_transp (const pixel_t* src, pixel_t* dest, int couleur)
{
  int x, y;

  for (y = 10; y != 0; y--, src += 320 - 12, dest += xbuf - 12)
    for (x = 12; x != 0; x--, src++, dest++)
      if (*src != 0)
	*dest = glenz[couleur][*dest];
}

static void
draw_dead_lemming (pixel_t* dest_, const lemming_t* ptibptr)
{
  pixel_t *dest;
  char d;

  if (ptibptr >= lemmings_support
      && ptibptr < lemmings_support + lemmings_total)
    do {
      dest = dest_;
      d = ptibptr->way;
      if (d == w_up)
	dest -= (ptibptr->min / 6553) * xbuf;
      if (d == w_right)
	dest += ptibptr->min / 5461;
      if (d == w_down)
	dest += (ptibptr->min / 6553) * xbuf;
      if (d == w_left)
	dest -= ptibptr->min / 5461;

      copy_dead_lemming_transp (vehicles_img.buffer + 181 * 320 - 16 +
				(ptibptr->dead << 4), dest,
				6 - (ptibptr->couleur));
      ptibptr = (lemming_t *) ptibptr->nexttache;
    } while (ptibptr >= lemmings_support
	     && ptibptr < lemmings_support + lemmings_total);
}


void
draw_level (int p)
{
  int i, j;
  int k, l;
  const lemming_t* tmppti;
  signed char bb;
  unsigned char b;
  pixel_t *dest = render_buffer[p] + sbuf;
  pixel_t *dest2;
  long anim_frame;
  int camera_stop_x[2];
  int camera_stop_y[2];

  clock_anim = clocks[read_htimer (clock_htimer) & 7];
  lemmings_anim_offset = (lemmings_move_offset * 64 / 65536) & 7 << 3;
  if (read_htimer (blink_htimer) & 1)
    for (bb = 3; bb >= 0; bb--)
      invincible[bb] = (player[bb].invincible != 0);
  else
    for (bb = 3; bb >= 0; bb--)
      invincible[bb] = 0;

  bonus_anim_offset = read_htimer (bonus_anim_htimer) % 25;
  if (bonus_anim_offset >= 13)
    bonus_anim_offset = 25 - bonus_anim_offset;

  if (lvl.tile_height_wrap == DONT_WRAP
      && (corner_dy[p] + 11U) > lvl.tile_height)
    camera_stop_y[p] = 1;
  else
    camera_stop_y[p] = 0;
  if (lvl.tile_width_wrap == DONT_WRAP
      && (corner_dx[p] + nbr_tiles_cols) > lvl.tile_width)
    camera_stop_x[p] = 1;
  else
    camera_stop_x[p] = 0;

  /* Draw background tiles */

  anim_frame = read_htimer (tiles_anim_htimer);

  for (k = corner_dy[p], l = 11 - camera_stop_y[p]; l > 0; l--, k++) {
    tile_index_t m;
    k = k & lvl.tile_height_wrap;
    m = k * lvl.tile_width;
    for (i = corner_dx[p], j = nbr_tiles_cols - camera_stop_x[p]; j > 0;
	 j--, i++) {
      i = (i & lvl.tile_width_wrap);
      if ((i + m) < lvl.tile_count) {
	bg_data_t* tile = bg_data + i + m;
	switch (tile->kind) {
	case A_NONE:
	  copy_tile (tile->source, dest, tile_set_img.width);
	  break;
	case A_LOOP:
	  copy_tile (tile->source +
		     24 * ((anim_frame/tile->anim_speed) % tile->anim_frames),
		     dest, tile_set_img.width);
	  break;
	case A_PINGPONG:
	  {
	    int frm = ((anim_frame / tile->anim_speed)
		       % (tile->anim_frames * 2));
	    if (frm > tile->anim_frames)
	      frm = 2 * tile->anim_frames - frm;
	    copy_tile (tile->source + 24 * frm, dest, tile_set_img.width);
	  }
	  break;
	}
      }
      dest += 24;
    }
    dest += xbuf * 20 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
  }

  /* draw bloody dead lemings */

  if (game_mode == M_KILLEM) {
    dest = render_buffer[p] + sbuf - 24 - 10 * xbuf;
    for (k = corner_dy[p] * 2 - 1, l = 2 + (11 - camera_stop_y[p]) * 2; l > 0;
	 l--, k++) {
      k &= lvl.square_height_wrap;
      if ((unsigned) k < lvl.square_height) {
	tile_index_t m;
	m = k * lvl.square_width;
	for (i = corner_dx[p] * 2 - 2, j =
	     2 + (nbr_tiles_cols - camera_stop_x[p]); j > 0; j--, i += 2) {
	  i &= lvl.square_width_wrap;
	  if ((unsigned) i < lvl.square_width) {
	    tmppti = square_dead_lemmings_list[i + m];
	    if (tmppti != NULL) {
	      draw_dead_lemming (dest, tmppti);
	    }
	    tmppti = square_dead_lemmings_list[i + m + 1];
	    if (tmppti != NULL) {
	      draw_dead_lemming (dest + 12, tmppti);
	    }
	  }
	  dest += 24;
	}
	dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]) - 24 * 2;
      } else
	dest += xbuf * 10;
    }

    /* draw lemmings */

    dest = render_buffer[p] + sbuf + 3 + xbuf;
    for (k = corner_dy[p] * 2, l = (11 - camera_stop_y[p]) * 2; l > 0;
	 l--, k = ((k + 1) & lvl.square_height_wrap)) {
      tile_index_t m;
      m = k * lvl.square_width;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & lvl.square_width_wrap)) {
	tmppti = square_lemmings_list[i + m];
	if (tmppti != NULL) {
	  draw_lemming (dest, tmppti, i + m);
	}
	tmppti = square_lemmings_list[i + m + 1];
	if (tmppti != NULL) {
	  draw_lemming (dest + 12, tmppti, i + m + 1);
	}
	dest += 24;
      }
      dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
    }
  }

  /* draw color pyramids */

  if (game_mode == M_COLOR) {
    dest = render_buffer[p] + sbuf + 2 + xbuf * 2;
    for (k = corner_dy[p] * 2, l = (11 - camera_stop_y[p]) * 2; l > 0;
	 l--, k = ((k + 1) & lvl.square_height_wrap)) {
      tile_index_t m;
      m = k * lvl.square_width;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & lvl.square_width_wrap)) {
	bb = square_object[i + m];
	if (bb >= 0)
	  draw_color (dest, bb);
	bb = square_object[i + m + 1];
	if (bb >= 0)
	  draw_color (dest + 12, bb);
	dest += 24;
      }
      dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
    }
  }

  /* draw small dollars */

  if (game_mode == M_TCASH) {
    dest = render_buffer[p] + sbuf + 2;
    for (k = corner_dy[p] * 2, l = (11 - camera_stop_y[p]) * 2; l > 0;
	 l--, k = ((k + 1) & lvl.square_height_wrap)) {
      tile_index_t m;
      m = k * lvl.square_width;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & lvl.square_width_wrap)) {
	bb = square_object[i + m];
	if (bb == 15)
	  DRAW_SPRITE (clock_anim, dest);
	else if (bb >= 0)
	  DRAW_SPRITE (small_dollar, dest);
	bb = square_object[i + m + 1];
	if (bb == 15)
	  DRAW_SPRITE (clock_anim, dest + 12);
	else if (bb >= 0)
	  DRAW_SPRITE (small_dollar, dest + 12);
	dest += 24;
      }
      dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
    }
  }

  /* draw transparent trails */

  dest = render_buffer[p] + sbuf;
  for (k = corner_dy[p] * 2, l = (11 - camera_stop_y[p]) * 2; l > 0;
       l--, k = ((k + 1) & lvl.square_height_wrap)) {
    tile_index_t m;
    m = k * lvl.square_width;
    for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]); j > 0;
	 j--, i = ((i + 2) & lvl.square_width_wrap)) {
      bb = square_occupied[i + m];
      if (bb != -1) {
	if (bb >= 0 && bb < 4)
	  draw_vehicle_tail (bb, dest);
	else if (bb >= 4 && bb < 8)
	  draw_vehicle_head ((char) (bb - 4), dest);
	else if (bb >= 8 && bb < 12)
	  draw_trail ((char) (bb - 6), dest, square_way[i + m]);
	else if (bb >= 12 && bb < 16)
	  draw_trail_tail ((char) (bb - 10), dest);
      }
      bb = square_occupied[i + m + 1];
      if (bb != -1) {
	dest2 = dest + 12;
	if (bb >= 0 && bb < 4)
	  draw_vehicle_tail (bb, dest2);
	else if (bb >= 4 && bb < 8)
	  draw_vehicle_head ((char) (bb - 4), dest2);
	else if (bb >= 8 && bb < 12)
	  draw_trail ((char) (bb - 6), dest2, square_way[i + m + 1]);
	else if (bb >= 12 && bb < 16)
	  draw_trail_tail ((char) (bb - 10), dest2);
      }
      dest += 24;
    }
    dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
  }

  /* Draw transparent sprites (trees...), and bonuses. */

  dest = render_buffer[p] + sbuf;
  for (k = corner_dy[p], l = 11 - camera_stop_y[p]; l > 0;
       l--, k = ((k + 1) & lvl.tile_height_wrap)) {
    tile_index_t m;
    m = k * lvl.tile_width;
    for (i = corner_dx[p], j = nbr_tiles_cols - camera_stop_x[p]; j > 0;
	 j--, i = ((i + 1) & lvl.tile_width_wrap)) {
      int pos = i + m;
      if (fg_data[pos].bonus)
	DRAW_SPRITE (fg_data[pos].bonus[bonus_anim_offset], dest);
      if (fg_data[pos].big_dollar)
	DRAW_SPRITE (big_dollar, dest + 4 + 2 * xbuf);
      if (fg_data[pos].sprite)
	DRAW_SPRITE (fg_data[pos].sprite, dest);
      dest += 24;
    }
    dest += xbuf * 20 - 24 * (nbr_tiles_cols - camera_stop_x[p]);
  }

  /* Draw explosions */

  dest = render_buffer[p] + sbuf - 12 - 11 * xbuf - 20 * xbuf - 12;
  for (k = corner_dy[p] * 2 - 2, l = 0; l != 4 + (11 - camera_stop_y[p]) * 2;
       l++, k++) {
    k &= lvl.square_height_wrap;
    if (((unsigned) k) < lvl.square_height) {
      tile_index_t m;
      m = k * lvl.square_width;
      for (i = corner_dx[p] * 2 - 1, j = 0;
	   (unsigned)j != 2 + (nbr_tiles_cols - camera_stop_x[p]) * 2;
	   j++, i++) {
	i &= lvl.square_width_wrap;
	if (((unsigned) i) < lvl.square_width) {
	  b = square_explo_state[m + i];
	  if (b <= EXPLOSION_IMMEDIATE)
	    DRAW_SPRITE (explosions[square_explo_type[m + i]][b], dest);
	}
	dest += 12;
      }
      dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]) - 2 * 12;
    } else
      dest += xbuf * 10;
  }

  /* Draw tutorial arrows */

  if (tutor) {
    int bonus_to_show = trail_size[col2plr[p]] < 55 ? 1 : 12;
    long wavepos = read_htimer (waving_htimer);
    /* Angle is the angle of the tail of the arrow, i.e.
       `<-' is 0 and `->' is M_PI.
       1 rotation per second.  */
    double angle = M_PI_4 - (wavepos * 2.0 * M_PI / 70.0);
    int arrow_index = (wavepos * NBR_ARROW_FRAMES / 70) % NBR_ARROW_FRAMES;
    sprite_t *s = tutorial_arrow[arrow_index];
    double d  = 10.0 + 4.0 * sin (wavepos / 40.0); /* arbitrary */
    /* Substract M_PI_4 so the arrow looks a bit inclinded.  */
    int dx =   d * cos (angle - M_PI_4);
    int dy = - d * sin (angle - M_PI_4);
    int color = minisinus[wavepos & 31] + 10;

    dest = render_buffer[p] + sbuf + (dy - 4) * xbuf + dx - 25;

    for (k = corner_dy[p] - 0, l = 1 + 11 - camera_stop_y[p]; l > 0;
	 l--, k++) {
      tile_index_t m;
      k &= lvl.tile_height_wrap;
      m = k * lvl.tile_width;
      for (i = corner_dx[p] - 1, j = 2 + nbr_tiles_cols - camera_stop_x[p];
	   j > 0; j--, i++) {
	i &= lvl.tile_width_wrap;
	if (tile_bonus[i + m] == bonus_to_show)
	  draw_sprunish_custom (s, dest, color);
	dest += 24;
      }
      dest += xbuf * 20 - 24 * (2 + nbr_tiles_cols - camera_stop_x[p]);
    }
  }
}


/* colors used to draw trails on the radar (the indice is the value
   of the square_occupied array) */
static const pixel_t radar_trail_color[16] = {
  111, 127, 143, 159,		/* vehicle head */
  111, 127, 143, 159,		/* vehicle tail */
  109, 125, 141, 157,		/* trail */
  109, 125, 141, 157		/* trail tail */
};

/* colors use to draw walls on the radar (the indice is the value
   if the square_wall array) */
static const pixel_t radar_wall_color[16] = {
  0, 89, 89, 91,
  89, 91, 91, 93,
  89, 91, 91, 93,
  91, 93, 93, 95
};

static void
draw_radar_frame (pixel_t *dest)
{
  unsigned int y;

  /* Top horizontal line. */
  memset (dest, 15, 75);
  dest += xbuf;
  /* Left and right lines.  */
  for (y = 40; y != 0; --y) {
    *dest = 15;
    dest[74] = 15;
    dest += xbuf;
  }
  /* Bottom line.  */
  memset (dest, 15, 75);
}

void
draw_radar_map (square_coord_t dx, square_coord_t dy, int radar_shift)
{
  pixel_t *src = corner[0] + 5 * xbuf + 239 + radar_shift;
  unsigned int x, y, tdym;
  square_coord_t tdx, tdy;
  signed char tmp;
  long blink = read_htimer (blink_htimer) & 2;

  /* If the radar is completly off the screen,
     don't bother drawing anything.  */
  if (radar_shift >= 81)
    return;

  draw_radar_frame (src);
  src += xbuf + 1;

  /* Now, draw the radar body.  */
  tdy = dy - 20;
  for (y = 40; y; --y) {
    tdy &= lvl.square_height_wrap;
    if (tdy < lvl.square_height) {
      tdym = tdy * lvl.square_width;
      tdx = dx - 36;
      for (x = 73; x; --x) {
	tdx &= lvl.square_width_wrap;
	if (tdx < lvl.square_width) {
	  tmp = tile_bonus[square_tile[tdx + tdym]];
	  /* If there is a bonus on this square ... */
	  if (tmp != 0 && tmp != -1) {
	    /* ... draw it, possibly blinking.  */
	    if ((tmp & 127) == 1 && blink)
	      *src = 31;
	    else
	      *src = 27;
	  } else {
	    /* Otherwise, maybe there is someone?  */
	    if ((tmp = square_occupied[tdx + tdym]) != -1)
	      *src = radar_trail_color[tmp];
	    /* or just a wall?  */
	    else if ((tmp = lvl.square_walls_out[tdx + tdym]) != 0)
	      *src = radar_wall_color[tmp];
	    /* or nothing interesting.  */
	    else
	      *src = glenz[0][*src];
	  }
	} else {
	  /* The square is out of the level (because the level
	     is not width-wrapped).  */
	  *src = glenz[0][*src];
	}
	src++;
	tdx++;
      }
    } else {
      /* The square is out of the level (because the level is
	 not height-wrapped).  */
      for (x = 73; x; --x, ++src)
	*src = glenz[0][*src];
    }
    src += xbuf - 73;
    tdy++;
  }
}

void
draw_score (int c, int p, unsigned int offset)
{
  pixel_t* src = corner[p] + offset;
  pixel_t* tmp = src + 22 + 2 * xbuf;
  pixel_t* tmp2 = src + 25 + 5 * xbuf + 53 * xbuf;
  pixel_t* dest = src + 2 + 2 * xbuf;
  int x, y, i;
  char score[32];

  if (radar_current_pos > 60)
    return;

  for (y = ((game_mode < M_TCASH) ? 63 : 75); y != 0; y--) {
    for (x = 33; x != 0; --x, ++src)
      *src = glenz[0][*src];
    src += xbuf - 33;
  }
  sprintf (score, "%.6d", player[c].score_delta >> 2);
  for (i = 0; i < 6; i++) {
    src = main_font_img.buffer + 72 * 320 + (score[i] - '0') * 18;
    for (y = 9; y != 0; y--) {
      for (x = 18; x != 0; x--, src++, dest++)
	if (*src != 0)
	  *dest = *src;
      dest += xbuf - 18;
      src += 302;
    }
    dest += xbuf;
  }
  src =
    main_font_img.buffer + 50 * 320 +
    ((player[c].lifes < 11) ? player[c].lifes - 1 : 10) * 10;
  for (y = 11; y != 0; y--) {
    for (x = 9; x != 0; x--, src++, tmp++)
      if (*src != 0)
	*tmp = *src;
    tmp += xbuf - 9;
    src += 311;
  }
  tmp += xbuf;
  src = main_font_img.buffer + 72 * 320 + 184;
  for (y = 47; y != 0; y--) {
    for (x = 9; x != 0; x--, src++, tmp++)
      if (*src != 0)
	*tmp = *src;
      else
	*tmp = glenz[0][*tmp];
    tmp += xbuf - 9;
    src += 311;
  }
  x = player[c].turbo_level_delta * 41 / 1024;
  tmp2 -= xbuf;
  src = main_font_img.buffer + 72 * 320 + 184 + 9 + x * 320 - 320;
  for (y = x; y != 0; y--) {
    *tmp2++ = *src++;
    *tmp2++ = *src++;
    *tmp2++ = *src++;
    tmp2 -= 3 + xbuf;
    src -= 3 + 320;
  }
  if (game_mode >= M_TCASH) {
    sprintf (score, "%.3d", player[c].time / 70);
    for (i = 0; i < 3; i++) {
      src = main_font_img.buffer + 50 * 320 + (score[i] - '0') * 10;
      for (y = 11; y != 0; y--) {
	for (x = 9; x != 0; x--, src++, dest++)
	  if (*src != 0)
	    *dest = *src;
	dest += xbuf - 9;
	src += 320 - 9;
      }
      dest += 10 - 11 * xbuf;
    }
  }
}

void
draw_logo_info (int c, int nbr, pixel_t* dest)
{
  pixel_t* src = dest;
  pixel_t* tmp;
  pixel_t* tmp2;
  pixel_t* tmp3;
  int x, y;

  if (game_mode < M_TCASH) {
    for (x = 50; x != 0; --x, ++src)
      *src = glenz[0][*src];
    src += xbuf - 50;
    tmp = src + 1;
    tmp2 = src + 11;
    for (y = 11; y != 0; y--) {
      for (x = 21; x != 0; --x, ++src)
	*src = glenz[0][*src];
      *(src + 28) = glenz[0][*(src + 28)];
      src += xbuf - 21;
    }
    for (x = 50; x != 0; --x, ++src)
      *src = glenz[0][*src];
    DRAW_SPRITE (player_logo[c], dest + xbuf + 21);

    src = main_font_img.buffer + 50 * 320 + (nbr / 10) * 10;
    for (y = 11; y != 0; y--) {
      for (x = 9; x != 0; x--, src++, tmp++)
	if (*src != 0)
	  *tmp = *src;
      tmp += xbuf - 9;
      src += 311;
    }
    src = main_font_img.buffer + 50 * 320 + (nbr % 10) * 10;
    for (y = 11; y != 0; y--) {
      for (x = 9; x != 0; x--, src++, tmp2++)
	if (*src != 0)
	  *tmp2 = *src;
      tmp2 += xbuf - 9;
      src += 311;
    }
    if (nbr == 0)
      DRAW_SPRITE (red_cross[c], dest - 5 * xbuf + 20);
  } else {
    src -= 5;
    dest -= 5;
    for (x = 60; x != 0; --x, ++src)
      *src = glenz[0][*src];
    src += xbuf - 60;
    tmp = src + 1;
    tmp2 = src + 11;
    tmp3 = src + 21;
    for (y = 11; y != 0; y--) {
      for (x = 31; x != 0; --x, ++src)
	*src = glenz[0][*src];
      src[28] = glenz[0][src[28]];
      src += xbuf - 31;
    }
    for (x = 60; x != 0; --x, ++src)
      *src = glenz[0][*src];
    DRAW_SPRITE (player_logo[c], dest + xbuf + 31);

    src = main_font_img.buffer + 50 * 320 + (nbr / 100) * 10;
    for (y = 11; y != 0; y--) {
      for (x = 9; x != 0; x--, src++, tmp++)
	if (*src != 0)
	  *tmp = *src;
      tmp += xbuf - 9;
      src += 311;
    }
    src = main_font_img.buffer + 50 * 320 + ((nbr / 10) % 10) * 10;
    for (y = 11; y != 0; y--) {
      for (x = 9; x != 0; x--, src++, tmp2++)
	if (*src != 0)
	  *tmp2 = *src;
      tmp2 += xbuf - 9;
      src += 311;
    }
    src = main_font_img.buffer + 50 * 320 + (nbr % 10) * 10;
    for (y = 11; y != 0; y--) {
      for (x = 9; x != 0; x--, src++, tmp3++)
	if (*src != 0)
	  *tmp3 = *src;
      tmp3 += xbuf - 9;
      src += 311;
    }
    if (player[c].spec == 0xde)
      DRAW_SPRITE (red_cross[c], dest - 5 * xbuf + 20);
  }
}
