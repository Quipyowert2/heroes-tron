/*------------------------------------------------------------------------.
| Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>                 |
|                                                                         |
| This file is part of Heroes.                                            |
|                                                                         |
| Heroes is free software; you can redistribute it and/or modify it under |
| the terms of the GNU General Public License version 2 as published by   |
| the Free Software Foundation.                                           |
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
#include "bonus.h"
#include "renderdata.h"
#include "const.h"
#include "heroes.h"
#include "sfx.h"
#include "argv.h"
#include "debugmsg.h"
#include "sprrle.h"
#include "sprtext.h"
#include "state.h"

static a_pcx_image bonus_a_img, bonus_b_img;

extern a_sprite *txt_bonus[4];	/* FIXME: get rid of this ! */

a_sprite *bonus_rle[2][N_BONUSES][N_BONUS_FRAMES];

unsigned char *tile_bonus;
unsigned char *tile_bonus_cpu;

a_timer bonus_anim_htimer;
int bonus_anim_offset;

a_sprite *txt_bonus[4] = { 0, 0, 0, 0 };
int txt_bonus_tempo[4];

void
init_bonuses (void)
{
  int bonus;
  int frame;

  bonus_anim_htimer = new_htimer (T_GLOBAL, HZ (35));
  pcx_load_from_rsc ("purple-bonus-img", &bonus_a_img);
  pcx_load_from_rsc ("brown-bonus-img", &bonus_b_img);
  pcx_load_from_rsc ("bonus-font", &bonus_font_img);

  for (bonus = 0; bonus < N_BONUSES; ++bonus)
    for (frame = 0; frame < N_BONUS_FRAMES; ++frame)
      bonus_rle[0][bonus][frame] =
	compile_sprrle (IMGPOS (bonus_a_img, bonus * 20, frame * 24),
			0, 20, 24, bonus_a_img.width, xbuf);
  for (bonus = 0; bonus < N_BONUSES; ++bonus)
    for (frame = 0; frame < N_BONUS_FRAMES; ++frame)
      bonus_rle[1][bonus][frame] =
	compile_sprrle (IMGPOS (bonus_b_img, bonus * 20, frame * 24),
			0, 20, 24, bonus_b_img.width, xbuf);

  img_free (&bonus_b_img);
  img_free (&bonus_a_img);
}

void
render_init_bonus_level (void)
{
  txt_bonus_tempo[0] = 0;
  txt_bonus_tempo[1] = 0;
  txt_bonus_tempo[2] = 0;
  txt_bonus_tempo[3] = 0;
}

void
uninit_bonuses (void)
{
  int bonus;
  int frame;

  img_free (&bonus_font_img);
  free_htimer (bonus_anim_htimer);

  for (bonus = 0; bonus < N_BONUSES; ++bonus)
    for (frame = 0; frame < N_BONUS_FRAMES; ++frame) {
      free_sprite (bonus_rle[0][bonus][frame]);
      free_sprite (bonus_rle[1][bonus][frame]);
    }
}

void
show_txt_bonus (int pl, a_pixel *dest)
{
  if (txt_bonus_tempo[pl] > 0)
    DRAW_SPRITE (txt_bonus[pl], dest);
}

void
set_txt_bonus (int pl, const char *txt, int tempo)
{
  FREE_SPRITE0 (txt_bonus[pl]);
  txt_bonus[pl] = compile_bonus_text (txt, T_FLUSHED_LEFT | T_WAVING, 0, 0);
  txt_bonus_tempo[pl] = tempo;
}

/* FIXME: Use a timer instead of calling this function.  */
void
update_player_bonus_vars (int pl)
{
  if (txt_bonus_tempo[pl] > 0)
    txt_bonus_tempo[pl]--;
}
