/*------------------------------------------------------------------------.
| Copyright (C) 2000 Alexandre Duret-Lutz <duret_g@epita.fr>              |
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

#ifndef HEROES__FONTDATA__H
#define HEROES__FONTDATA__H

#include "display.h"

typedef struct {
  unsigned int	width[256];	/* width of each character */
  unsigned int	height;		/* height common to each character */
  pixel_t*	upper_left[256]; /* upper left pixel of each character.
				    DO NOT use this pointer if the 
				    corresponding width is null. */
  unsigned int	line_size;	/* size of a line in the font buffer */
} fontdata_t;

extern fontdata_t *edit_font;
extern fontdata_t *menu_font;
extern fontdata_t *help_font;
extern fontdata_t *deck_font;

void init_fonts (void);
void uninit_fonts (void);

/* Compute the width needed by `text' using font `font'.
   If ignore_spaces is non null, spaces are not accounted. */
unsigned int compute_text_width (const fontdata_t *font, const char *text,
				 int ignore_spaces);

#endif /* HEROES__FONTDATA__H */
