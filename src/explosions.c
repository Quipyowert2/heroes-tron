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
#include "explosions.h"
#include "sprzcol.h"
#include "pcx.h"
#include "const.h"

a_sprite *explosions[NBR_EXPLOSION_KINDS][NBR_EXPLOSION_FRAMES];

void
init_explosions (void)
{
  int i;
  for (i = 0; i < 6; ++i)
    explosions[0][i] = compile_sprzcol (IMGPOS (vehicles_img, 65, (5 - i)*33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (; i < NBR_EXPLOSION_FRAMES; ++i)
    explosions[0][i] = compile_sprzcol (IMGPOS (vehicles_img, 32,
						(NBR_EXPLOSION_FRAMES - i - 1)
						* 33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (i = 0; i < 6; ++i)
    explosions[1][i] = compile_sprzcol (IMGPOS (vehicles_img, 131, (5 - i)*33),
					0, 32, 32, vehicles_img.width, xbuf);
  for (; i < NBR_EXPLOSION_FRAMES; ++i)
    explosions[1][i] = compile_sprzcol (IMGPOS (vehicles_img, 98,
						(NBR_EXPLOSION_FRAMES - i - 1)
						* 33),
					0, 32, 32, vehicles_img.width, xbuf);
}

void
uninit_explosions (void)
{
  int i, j;
  for (i = 0; i < NBR_EXPLOSION_KINDS; ++i)
    for (j = 0; j < NBR_EXPLOSION_FRAMES; ++j)
      FREE_SPRITE0 (explosions[i][j]);
}
