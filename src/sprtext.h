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

#ifndef HEROES__SPRTEXT__H
#define HEROES__SPRTEXT__H

#include "sprite.h"
#include "fontdata.h"


/* Don't change the order of the four first options.  The code rely
   on these values (for instance we must have
   T_FLUSHED_RIGHT | T_FLUSHED_LEFT == T_JUSTIFIED). */
enum text_option { T_CENTERED = 0,
		   T_FLUSHED_LEFT = 1,
		   T_FLUSHED_RIGHT = 2,
		   T_JUSTIFIED = 3,
		   T_WAVING = 4};

sprite_t *compile_sprtext (const fontdata_t *font, const char *text,
			   enum text_option topt, unsigned int maxwidth,
			   int offset);

sprite_t *compile_sprpara (const fontdata_t *font, const char *text,
			   enum text_option topt, unsigned int maxwidth,
			   int offset);

/* short-hand functions */

sprite_t *compile_menu_text (const char *text, enum text_option topt,
			     int row, int col);

sprite_t *compile_menu_para (const char *text, enum text_option topt,
			     int row, int col, int maxwidth);

sprite_t *compile_deck_text (const char *text, enum text_option topt,
			     int row, int col);

sprite_t *compile_bonus_text (const char *text, enum text_option topt,
			      int row, int col);

#endif /* HEROES__SPRTEXT__H */
