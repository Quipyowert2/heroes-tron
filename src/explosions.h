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

#ifndef HEROES__EXPLOSIONS__H
#define HEROES__EXPLOSIONS__H

#include "sprite.h"
#include "lvl/lvl.h"

#define NBR_EXPLOSION_FRAMES 15
#define NBR_EXPLOSION_KINDS   2

/* These are the sprites used to display
   each frame of each kind of explosion.  */
extern sprite_t *explosions[NBR_EXPLOSION_KINDS][NBR_EXPLOSION_FRAMES];

#define EXPLOSION_SLICES_PER_FRAMES 8
#define EXPLOSION_DELAY 8

#define EXPLOSION_IMMEDIATE (NBR_EXPLOSION_FRAMES - 1)
#define EXPLOSION_TRIGGERED (EXPLOSION_IMMEDIATE + EXPLOSION_DELAY)
#define EXPLOSION_TRIGGER_NEIGHBORS (EXPLOSION_TRIGGERED - 3)
#define EXPLOSION_UNTRIGGERED (EXPLOSION_TRIGGERED + 1)

/*
 * 0 <= n < NBR_EXPLOSION_FRAMES: frame number to display.
 * NBR_EXPLOSION_FRAMES <= n <= EXPLOSION_TRIGGERED: about to explode.
 * n == EXPLOSION_UNTRIGGERED: idle.
 */
typedef u8_t explosion_t;

extern explosion_t *square_explo_state;
extern explosion_t *square_explo_type;

void allocate_explosions (void);
void release_explosions (void);
/* FRAME_START is expected to be EXPLOSION_IMMEDIATE or EXPLOSION_TRIGGERED. */
void trigger_explosion (square_index_t idx, unsigned frame_start);
void trigger_possible_explosion (square_index_t idx);
void update_explosions (void);

void init_explosions (void);
void uninit_explosions (void);

#endif /* HEROES__EXPLOSIONS__H */
