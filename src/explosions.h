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

#ifndef HEROES__EXPLOSIONS__H
#define HEROES__EXPLOSIONS__H

#include "sprite.h"
#include "lvl.h"
#include "state.h"

/* These are the sprites used to display
   each frame of each kind of explosion.  */
extern a_sprite *explosions[NBR_EXPLOSION_KINDS][NBR_EXPLOSION_FRAMES];

void init_explosions (void);
void uninit_explosions (void);

#endif /* HEROES__EXPLOSIONS__H */
