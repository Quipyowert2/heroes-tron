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
#include "items.h"
#include "const.h"
#include "sprshade.h"

sprite_t *big_dollar = 0;
sprite_t *small_dollar = 0;
sprite_t *clocks[NBR_CLOCK_FRAMES];
sprite_t *pyramids[NBR_PYRAMIDS];

void
init_items (void)
{
  int i;

  big_dollar = compile_sprshade (IMGPOS (main_font_img, 81, 0),
				 0, 1, glenz[0],
				 17, 17, main_font_img.width, xbuf);
  small_dollar = compile_sprshade (IMGPOS (main_font_img, 81, 18),
				   0, 1, glenz[0],
				   10, 10, main_font_img.width, xbuf);
  for (i = 0; i < NBR_CLOCK_FRAMES; ++i)
    clocks[i] = compile_sprshade (IMGPOS (main_font_img, 81, 52 + i * 10),
				  0, 1, glenz[0],
				  10, 10, main_font_img.width, xbuf);
  for (i = 0; i < NBR_PYRAMIDS; ++i)
    pyramids[i] = compile_sprshade (IMGPOS (main_font_img, 64, i * 16),
				    0, 1, glenz[0],
				    7, 9, main_font_img.width, xbuf);
}

void
uninit_items (void)
{
  int i;
  FREE_SPRITE0 (big_dollar);
  FREE_SPRITE0 (small_dollar);
  for (i = 0; i < NBR_CLOCK_FRAMES; ++i)
    FREE_SPRITE0 (clocks[i]);
  for (i = 0; i < NBR_PYRAMIDS; ++i)
    FREE_SPRITE0 (pyramids[i]);
}
