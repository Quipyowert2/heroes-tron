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

#ifndef HEROES__BONUSES__H
#define HEROES__BONUSES__H

/** -- BEGIN PUBLIC -- **/

typedef enum a_bonus a_bonus;
typedef a_u8 a_bonus8;		/* Usually, we store bonus on 8bits. */

enum a_bonus {
  B_NOTHING,

  B_LENGTH_UP,			/* L+ */
  B_LENGTH_DOWN,		/* L- */
  B_SPEED_UP,			/* S+ */
  B_SPEED_DOWN,			/* S- */
  B_RANDOM,			/* ? */
  B_POINTS,			/* C (probably this stood for `Coins') */
  B_FIRE_TRAIL,			/* ZZ */
  B_PAUSE,			/* !! */
  B_INVERTED,			/* -1 */
  B_TURBO_UP,			/* T+ */
  B_TURBO_DOWN,			/* T- */
  B_END_OF_LEVEL,		/* EL */
  B_INVINCIBLE,			/* [] */
  B_ROTOZOOM,			/* X */
  B_EXTRA_LIFE,			/* XL */
  B_WAVES,			/* ~~ */
  B__UNUSED_YET,

  /* This one is not drawn in bonus{a,b}.pcx because it's not really
     bonus, but we handle it like the other bonus.  */
  B_BIG_DOLLAR,

  /* `|' this one with the above values to yellow bonuses instead of
     purple bonuses.  */
  B_YELLOW = 128,

  /* Indicate that this place should never contain a bonus.  */
  B_NOT_HERE = 255,
};

#define BONUS_TYPE(b)  ((b) & 127)
#define BONUS_YELLOW_P(b) (((b) & B_YELLOW) == B_YELLOW)
#define BONUS_P(b) ((b) != B_NOTHING && (b) != B_NOT_HERE)

/** -- END PUBLIC -- **/

#endif /* HEROES__BONUSES__H */
