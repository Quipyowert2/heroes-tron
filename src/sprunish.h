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

#ifndef HEROES__SPRUNISH__H
#define HEROES__SPRUNISH__H

/*---------------------------------------------------------------.
| transparent sprites that use one color key for glenz and one   |
| unique colore for opaque pixel.  This is used to draw the "Get |
| this bonus" arrow.                                             |
`---------------------------------------------------------------*/

#include "sprite.h"

void draw_sprunish (const sprite_t *sprite, pixel_t *dest);
void draw_sprunish_custom (const sprite_t *sprite, pixel_t *dest,
			   pixel_t color);

sprite_t *compile_sprunish (const pixel_t *src, pixel_t transp_color,
			    pixel_t glenz_color, pixel_t *glenz_line,
			    pixel_t opaque_color,
			    unsigned int block_height,
			    unsigned int block_width,
			    unsigned int src_width, unsigned int dest_width);

void free_sprunish (sprite_t *prog);

#endif /* HEROES__SPRSHADE__H */
