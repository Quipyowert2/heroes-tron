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

#ifndef HEROES__SPRGLENZ__H
#define HEROES__SPRGLENZ__H

/*------------------------------------------------------------------.
| transparent sprites whose non-transparent pixel are drawn using a |
| glenz line.  This is used for trails, amongst other things.       |
`------------------------------------------------------------------*/

#include "sprrle.h"

void draw_sprglenz (const sprite_t *sprite, pixel_t *dest);

/* Same as above, but allow to chose the glenz line used */
void draw_sprglenz_custom (const sprite_t *sprite, pixel_t *dest,
			   const pixel_t *glenz_line);

sprite_t *compile_sprglenz (const pixel_t *src, pixel_t transp_color,
			    pixel_t *glenz_line,
			    unsigned int block_height,
			    unsigned int block_width,
			    unsigned int src_width, unsigned int dest_width);

void free_sprglenz (sprite_t *prog);

#endif /* HEROES__SPRGLENZ__H */
