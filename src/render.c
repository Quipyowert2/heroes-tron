/*------------------------------------------------------------------------.
| Copyright 1997, 1998, 2000  Alexandre Duret-Lutz <duret_g@epita.fr>     |
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
#include "display.h"
#include "const.h"
#include "options.h"
#include "fastmem.h"
#include "draw.h"
#include "render.h"
#include "argv.h"
#include "timer.h"
#include "heroes.h"
#include "renderdata.h"
#include "bonus.h"
#include "explosions.h"
#include "items.h"

char tutor = 0;
static sprite_t *clock_anim;

static void
copy_tile (const pixel_t* src, pixel_t* dest, int tx)
{
  const int *s = (const int *) src;
  int *d = (int *) dest;
  int t1, t2, y;
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
    s = (const int *) (((int) s) + tx);
    d = (int *) (((int) d) + xbuf);
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
    dest += 384 - 12 + d;
  }
}

static void
draw_trail_real (int c, unsigned char s, pixel_t* dest,
		 unsigned char fixe)
{
  const pixel_t* src;
  pixel_t ch;
  const pixel_t* glenzline;
  int d = 0, x, y;

  if (fixe) {
    if (s & 1)
      d = player[c].d.e / 5461;
    else
      d = player[c].d.e / 6554;
  }
  src = (const pixel_t *) trail[s] + (d << 4);
  if (opt.use_glenz) {
    glenzline = glenz[c + 2];
    for (y = 10; y != 0; y--) {
      for (x = 12; x != 0; x--) {
	ch = *src++;
	if (ch != 0)
	  *dest = glenzline[*dest];
	dest++;
      }
      dest += xbuf - 12;
      src += 192 - 12;
    }
  } else {
    pixel_t col = (pixel_t) (NOGLENZPLR + (c << 4));
    for (y = 10; y != 0; y--) {
      for (x = 12; x != 0; x--) {
	ch = *src++;
	if (ch != 0)
	  *dest = col;
	dest++;
      }
      dest += xbuf - 12;
      src += 192 - 12;
    }
  }
}

static void
draw_vehicle_tail (int c, pixel_t* dest)
{
  int d = 0, x, y;
  const pixel_t* posit = 0;
  int s = ((player[c].old_way ^ 2) + ((player[c].way ^ 2) << 2));
  const pixel_t* src;
  pixel_t* tmp = dest;
  const pixel_t* glenzline;
  pixel_t ch, cc;

  if (s & 4) {
    d = player[c].d.e / 5461;
    src = (const pixel_t *) trail[s] + ((12 - (d + 12) / 2) << 4);
  } else {
    d = player[c].d.e / 6554;
    src = (const pixel_t *) trail[s] + ((10 - (d + 10) / 2) << 4);
  }
  if (opt.use_glenz) {
    glenzline = glenz[c + 2];
    for (y = 10; y != 0; y--) {
      for (x = 12; x != 0; x--) {
	ch = *src++;
	if (ch != 0)
	  *dest = glenzline[*dest];
	dest++;
      }
      dest += xbuf - 12;
      src += 192 - 12;
    }
  } else {
    cc = (pixel_t) (NOGLENZPLR + (c << 4));
    for (y = 10; y != 0; y--) {
      for (x = 12; x != 0; x--) {
	ch = *src++;
	if (ch != 0)
	  *dest = cc;
	dest++;
      }
      dest += xbuf - 12;
      src += 192 - 12;
    }
  }
  posit = vehicles_img.buffer + (c << 6) + (player[c].way << 4);
  if (invincible[c])
    posit += 10 * 320;
  if (player[c].way == w_left)
    copy_square_transp (posit + d, tmp, (char) d, 0);
  if (player[c].way == w_right)
    copy_square_transp (posit, tmp + d, (char) d, 0);
  if (player[c].way == w_up)
    copy_square_transp (posit + d * 320, tmp, 0, (char) d);
  if (player[c].way == w_down)
    copy_square_transp (posit, tmp + d * xbuf, 0, (char) d);
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
  posit = vehicles_img.buffer + (c << 6) + (player[c].way << 4);
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
  if (opt.use_glenz)
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
  } else
    for (j = 8; j != 0; j--) {
      for (k = 8; k != 0; k--) {
	if (*src != 0 && *src != 1)
	  *dest = *src;
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
  if (opt.use_glenz) {
    for (y = 10; y != 0; y--, src += 320 - 12, dest += xbuf - 12)
      for (x = 12; x != 0; x--, src++, dest++)
	if (*src != 0)
	  *dest = glenz[couleur][*dest];
  } else {
    couleur = ((couleur == 6) ? NOGLENZRED : NOGLENZPLR + 16 * 3);
    for (y = 10; y != 0; y--, src += 320 - 12, dest += xbuf - 12)
      for (x = 12; x != 0; x--, src++, dest++)
	if (*src != 0)
	  *dest = couleur;
  }
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
  int k, l, m;
  const lemming_t* tmppti;
  signed char bb;
  unsigned char b;
  unsigned int ib;
  signed char sinl;
  pixel_t *dest = render_buffer[p] + sbuf;
  pixel_t *dest2;
  long anim_frame;

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

  if (map_info.ywrap == DONT_WRAP && (corner_dy[p] + 11U) > map_info.yt)
    camera_stop_y[p] = 1;
  else
    camera_stop_y[p] = 0;
  if (map_info.xwrap == DONT_WRAP &&
      (corner_dx[p] + nbr_tiles_cols) > map_info.xt)
    camera_stop_x[p] = 1;
  else
    camera_stop_x[p] = 0;

  /* Draw background tiles */

  anim_frame = read_htimer (tiles_anim_htimer);

  for (k = corner_dy[p], l = 11 - camera_stop_y[p]; l > 0; l--, k++) {
    k = k & map_info.ywrap;
    m = k * map_info.xt;
    for (i = corner_dx[p], j = nbr_tiles_cols - camera_stop_x[p]; j > 0;
	 j--, i++) {
      i = (i & map_info.xwrap);
      if ((i + m) < (int)(map_info.xt * map_info.yt)) {
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
      k &= map_info_2ywrap;
      if ((unsigned) k < map_info_2yt) {
	m = k * map_info_2xt;
	for (i = corner_dx[p] * 2 - 2, j =
	     2 + (nbr_tiles_cols - camera_stop_x[p]); j > 0; j--, i += 2) {
	  i &= map_info_2xwrap;
	  if ((unsigned) i < map_info_2xt) {
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
	 l--, k = ((k + 1) & (map_info_2ywrap))) {
      m = k * map_info_2xt;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & (map_info_2xwrap))) {
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
	 l--, k = ((k + 1) & (map_info_2ywrap))) {
      m = k * map_info_2xt;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & (map_info_2xwrap))) {
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
	 l--, k = ((k + 1) & (map_info_2ywrap))) {
      m = k * map_info_2xt;
      for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]);
	   j > 0; j--, i = ((i + 2) & (map_info_2xwrap))) {
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
       l--, k = ((k + 1) & (map_info_2ywrap))) {
    m = k * map_info_2xt;
    for (i = corner_dx[p] * 2, j = (nbr_tiles_cols - camera_stop_x[p]); j > 0;
	 j--, i = ((i + 2) & (map_info_2xwrap))) {
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
       l--, k = ((k + 1) & map_info.ywrap)) {
    m = k * map_info.xt;
    for (i = corner_dx[p], j = nbr_tiles_cols - camera_stop_x[p]; j > 0;
	 j--, i = ((i + 1) & map_info.xwrap)) {
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
    k &= map_info_2ywrap;
    if (((unsigned) k) < map_info_2yt) {
      m = k * map_info_2xt;
      for (i = corner_dx[p] * 2 - 1, j = 0;
	   (unsigned)j != 2 + (nbr_tiles_cols - camera_stop_x[p]) * 2;
	   j++, i++) {
	i &= map_info_2xwrap;
	if (((unsigned) i) < map_info_2xt) {
	  b = square_explosion[m + i];
	  if (b < (NBR_EXPLOSION_FRAMES - 1) * 8 - 1) {
	    b++;
	    DRAW_SPRITE (explosions[square_explosion_type[m + i]][b >> 3],
			 dest);
	  }
	}
	dest += 12;
      }
      dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]) - 2 * 12;
    } else
      dest += xbuf * 10;
  }

  /* Draw explosions from dead players */

  if ((unsigned) (event_time - last_explo) <
      (NBR_EXPLOSION_FRAMES - 1) * 8 - 1) {
    dest = render_buffer[p] + sbuf - 12 - 11 * xbuf - 20 * xbuf - 12;
    for (k = corner_dy[p] * 2 - 2, l = 0;
	 l != 4 + (11 - camera_stop_y[p]) * 2; l++, k++) {
      k &= map_info_2ywrap;
      if (((unsigned) k) < map_info_2yt) {
	m = k * map_info_2xt;
	for (i = corner_dx[p] * 2 - 1, j = 0;
	     (unsigned)j != 2 + (nbr_tiles_cols - camera_stop_x[p]) * 2;
	     j++, i++) {
	  i &= map_info_2xwrap;
	  if (((unsigned) i) < map_info_2xt) {
	    ib = event_time - square_dead_explosion[m + i];
	    if (ib < (NBR_EXPLOSION_FRAMES - 1) * 8 - 1) {
	      ib++;
	      DRAW_SPRITE (explosions[square_explosion_type[m + i]]
			   [NBR_EXPLOSION_FRAMES - 2 - (ib >> 3)], dest);
	    }
	  }
	  dest += 12;
	}
	dest += xbuf * 10 - 24 * (nbr_tiles_cols - camera_stop_x[p]) - 2 * 12;
      } else
	dest += xbuf * 10;
    }
  }

  /* Draw tutorial arrows */

  if (tutor) {
    sinl = (signed char) minisinus[read_htimer (waving_htimer) & 31];
    dest = render_buffer[p] + sbuf - (7 + sinl) * xbuf + 15 + sinl - 48;
    if (trail_size[col2plr[p]] < 55)
      for (k = corner_dy[p] - 0, l = 1 + 11 - camera_stop_y[p]; l > 0;
	   l--, k++) {
	k &= map_info.ywrap;
	m = k * map_info.xt;
	for (i = corner_dx[p] - 2, j = 2 + nbr_tiles_cols - camera_stop_x[p];
	     j > 0; j--, i++) {
	  i &= map_info.xwrap;
	  if (tile_bonus[i + m] == 1) {
	    if (j != 1)
	      copy_rect_transp_8 (main_font_img.buffer + 17 + 91 * 320, dest,
				  49, 13, 10 + sinl);
	    else
	      copy_rect_transp_8 (main_font_img.buffer + 17 + 91 * 320, dest,
				  20, 13, 10 + sinl);
	  }
	  dest += 24;
	}
	dest += xbuf * 20 - 24 * (2 + nbr_tiles_cols - camera_stop_x[p]);
    } else
      for (k = corner_dy[p] - 0, l = 1 + 11 - camera_stop_y[p]; l > 0;
	   l--, k++) {
	k &= map_info.ywrap;
	m = k * map_info.xt;
	for (i = corner_dx[p] - 2, j = 2 + nbr_tiles_cols - camera_stop_x[p];
	     j > 0; j--, i = (i + 1)) {
	  i &= map_info.xwrap;
	  if (tile_bonus[i + m] == 12) {
	    if (j != 1)
	      copy_rect_transp_8 (main_font_img.buffer + 17 + 91 * 320, dest,
				  49, 13, 10 + sinl);
	    else
	      copy_rect_transp_8 (main_font_img.buffer + 17 + 91 * 320, dest,
				  20, 13, 10 + sinl);
	  }
	  dest += 24;
	}
	dest += xbuf * 20 - 24 * (2 + nbr_tiles_cols - camera_stop_x[p]);
      }
  }

}



void
draw_radar_map (int dx, int dy)
{
  pixel_t *src = corner[0] + 5 * xbuf + 239 + radar_current_pos;
  int x, y, tdx, tdy, tdym, dede = 50 * (radar_current_pos > 60);
  signed char tmp;
  long blink = read_htimer (blink_htimer) & 2;

  if (radar_current_pos >= 81)
    return;
  for (x = 75 - dede; x != 0; x--)
    *src++ = 15;
  src += xbuf - 75 + dede;

  for (y = 40; y != 0; y--) {
    *src = 15;
    src[74 - dede] = 15;
    src += xbuf;
  }
  for (x = 75 - dede; x != 0; x--)
    *src++ = 15;

  src = corner[0] + 6 * xbuf + 240 + radar_current_pos;
  tdy = dy - 20;
  for (y = 40; y != 0; y--) {
    tdy &= map_info_2ywrap;
    if (tdy >= 0 && (tdy >> 1) < (int)map_info.yt) {
      tdym = tdy * map_info_2xt;
      tdx = dx - 36;
      for (x = 73 - dede; x != 0; x--) {
	tdx &= map_info_2xwrap;
	if (tdx >= 0 && (tdx >> 1) < (int)map_info.xt) {
	  tmp = tile_bonus[square2tile[tdx + tdym]];
	  if (tmp != 0 && tmp != -1) {
	    if ((tmp & 127) == 1 && blink)
	      *src = 31;
	    else
	      *src = 27;
	  } else if ((tmp = square_occupied[tdx + tdym]) != -1)
	    *src = radar_trail_color[tmp];
	  else if ((tmp = square_wall[tdx + tdym]) != 0)	/* square_radar_wall */
	    *src = radar_wall_color[tmp];
	  else
	    *src = glenz[0][*src];
	} else
	  *src = glenz[0][*src];
	src++;
	tdx++;
      }
    } else
      for (x = 73 - dede; x != 0; x--)
	*src++ = glenz[0][*src];
    src += xbuf - 73 + dede;
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
    for (x = 33; x != 0; x--)
      *src++ = glenz[0][*src];
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
    for (x = 50; x != 0; x--)
      *src++ = glenz[0][*src];
    src += xbuf - 50;
    tmp = src + 1;
    tmp2 = src + 11;
    for (y = 11; y != 0; y--) {
      for (x = 21; x != 0; x--)
	*src++ = glenz[0][*src];
      *(src + 28) = glenz[0][*(src + 28)];
      src += xbuf - 21;
    }
    for (x = 50; x != 0; x--)
      *src++ = glenz[0][*src];
    copy_rect_4 (main_font_img.buffer + 196 + c * 28 + 72 * 320,
		 dest + xbuf + 21, 28, 11);

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
      copy_rect_transp_red (main_font_img.buffer + 320 * 119 + (c << 6),
			    dest - 5 * xbuf + 10, 40, 22);
  } else {
    src -= 5;
    dest -= 5;
    for (x = 60; x != 0; x--)
      *src++ = glenz[0][*src];
    src += xbuf - 60;
    tmp = src + 1;
    tmp2 = src + 11;
    tmp3 = src + 21;
    for (y = 11; y != 0; y--) {
      for (x = 31; x != 0; x--)
	*src++ = glenz[0][*src];
      *(src + 28) = glenz[0][*(src + 28)];
      src += xbuf - 31;
    }
    for (x = 60; x != 0; x--)
      *src++ = glenz[0][*src];
    copy_rect_4 (main_font_img.buffer + 196 + c * 28 + 72 * 320,
		 dest + xbuf + 31, 28, 11);

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
    if (player[ /*plr2col[ */ c /*] */ ].spec == 0xde)
      copy_rect_transp_red (main_font_img.buffer + 320 * 119 + (c << 6),
			    dest - 5 * xbuf + 20, 40, 22);
  }
}

void
display_buffer_tmp1 (void)
{
  const pixel_t* src = render_buffer[1];
  pixel_t* dest = screen;
  int i;
  draw_demo_stick (src);
  for (i = 200; i > 0; i--, src += xbuf, dest += 320)
    fastmem4 (src, dest, 320 / 4);
}

void
display_buffer_moving (int x)
{
  const pixel_t* src = corner[0];
  pixel_t *dest = screen;
  int *desti;
  int i, j;
  draw_demo_stick (src);
  for (i = 200; i > 0; i--, src += xbuf, dest += 320) {
    fastmem4 (src + (x << 2), dest, 160 / 4 - x);
    desti = ((int *) dest) + 40 - x;
    for (j = (x << 1); j != 0; j--)
      *desti++ = 0;
    fastmem4 (src + 160, dest + 160 + (x << 2), 160 / 4 - x);
  }
}
void
display_two_buffers (void)
{
  const pixel_t* src1 = corner[swapside];
  const pixel_t* src2 = corner[1 - swapside];
  pixel_t *dest = screen;
  int i;
  draw_demo_stick (src2 - 160);
  for (i = 200; i > 0; i--, src1 += xbuf, src2 += xbuf, dest += 320) {
    fastmem4 (src1, dest, 160 / 4);
    fastmem4 (src2, dest + 160, 160 / 4);
  }
}

void
display_two_buffers_moving (int x)
{
  const pixel_t* src1 = corner[swapside];
  const pixel_t* src2 = corner[1 - swapside];
  pixel_t *dest = screen;

  int i;
  draw_demo_stick (src2 - 160);
  for (i = 200; i > 0; i--, src1 += xbuf, src2 += xbuf, dest += 320) {
    fastmem4 (src1 + (x << 2), dest, 160 / 4 - x);
    fastmem4 (src2, dest + 160 + (x << 2), 160 / 4 - x);
  }
}

/* this one clear the screen too */
void
display_two_buffers_moving_and_clear (int x)
{
  const pixel_t* src1 = corner[swapside];
  const pixel_t* src2 = corner[1 - swapside];
  pixel_t* dest = screen;
  int *desti;
  int i, j;
  draw_demo_stick (src2 - 160);
  for (i = 200; i > 0; i--, src1 += xbuf, src2 += xbuf, dest += 320) {
    fastmem4 (src1 + (x << 2), dest, 160 / 4 - x);
    desti = ((int *) dest) + 40 - x;
    for (j = (x << 1); j != 0; j--)
      *desti++ = 0;
    fastmem4 (src2, dest + 160 + (x << 2), 160 / 4 - x);
  }
}
