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
#include "sprtext.h"
#include "sprprogwav.h"
#include "sprrle.h"
#include "const.h"

/*
 * Generate a sprite_t that display a text, using a given font.
 *
 * Currently, an S_RLE sprite is computed for each character and chained
 * together in a S_PROG sprite.  This can be enhanced: unless waving,
 * there is no point in computing a S_RLE for each separate character,
 * it may be better to compute *one* S_RLE for the *whole* string.
 */

sprite_t*
compile_sprtext (const fontdata_t *font, const char *text,
		 enum text_option topt, unsigned int maxwidth,
		 int offset)
{
  if (topt & T_FLUSHED_LEFT) {	/* FLUSHED_LEFT or JUSTIFIED */
    /* offset = 0; */
  } else {			/* FLUSHED_RIGHT or CENTERED */
    unsigned int text_width = compute_text_width (font, text, 0);

    if (topt & T_FLUSHED_RIGHT)	/* FLUSHED_RIGHT */
      offset -= (int) text_width;
    else			/* CENTERED */
      offset -= (int) text_width/2;
  }

  new_sprprog ();

  for (; *text; ++text) {
    if (*text == ' ') {
      offset += font->width[' '];
      /* FIXME: handle the JUSTIFIED case here. */
    } else {
      add_sprprog (compile_sprrle (font->upper_left[(int)*text], 0,
				   font->height, font->width[(int)*text],
				   font->line_size, xbuf),
		   offset);
      offset += font->width[(int)*text];
    }
  }

  if (topt & T_WAVING)
    return end_sprprogwav ();
  else
    return end_sprprog ();
}

sprite_t*
compile_menu_text (const char *text, enum text_option topt,
		   int row, int col)
{
  return compile_sprtext (menu_font, text, topt, 0, row * xbuf + col);
}

sprite_t*
compile_deck_text (const char *text, enum text_option topt,
		   int row, int col)
{
  return compile_sprtext (deck_font, text, topt, 0, row * xbuf + col);
}

sprite_t *
compile_bonus_text (const char *text, enum text_option topt,
		    int row, int col)
{
  return compile_sprtext (bonus_font, text, topt, 0, row * xbuf + col);
}
