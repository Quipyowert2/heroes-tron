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

#include "common.h"
#include "rletext.h"
#include "const.h"
#include "debugmsg.h"
#include "draw.h"


/* used to alter the offset of waving strings */
static int
waving_offset (const rleprog_t* prog)
{
  return ceil (sin (((text_waving_step + prog->func_data*2) & 31)
		    * 3.141592653 / 16.0)
	       * 1.7) * prog->line_size;
}


/*
 * Generate a RLE-program that display a text, using a given font.
 *
 * Currently, rleprog_t are computed for each character and chained
 * together.  This can be enhanced: unless waving, there is no point
 * in computing a REL-prog for each character, it may be better to
 * compute *one* REL-prog for the *whole* string.
 */

rleprog_t*
compile_rletext (const fontdata_t *font, const char *text, 
		 enum text_option topt, unsigned int maxwidth,
		 int offset)
{
  rleprog_t *result = 0;	/* RLE-program to return */
  rleprog_t **next = &result;	/* place for the next RLE-program */

  if (topt & T_FLUSHED_LEFT) {	/* FLUSHED_LEFT or JUSTIFIED */
    /* offset = 0; */
  } else {			/* FLUSHED_RIGHT or CENTERED */
    unsigned int text_width = compute_text_width (font, text, 0);

    if (topt & T_FLUSHED_RIGHT)	/* FLUSHED_RIGHT */
      offset -= (int) text_width;
    else			/* CENTERED */
      offset -= (int) text_width/2;
  }

  for (; *text; ++text) {
    if (*text == ' ') {
      offset += font->width[' '];
      /* FIXME: handle the JUSTIFIED case here. */
    } else {
      *next = compile_rleprog (font->upper_left[(int)*text], 0,
			       font->height, font->width[(int)*text],
			       font->line_size, xbuf);
      (*next)->dest_offset = offset;
      result->latest_known = *next;
      next = &((*next)->next_prog);
      offset += font->width[(int)*text];
    }
  }

  /* setup waving parameters, if needed */
  if (topt & T_WAVING) {
    rleprog_t* cur = result;
    int number = 0;

    while (cur) {
      cur->func_offset = waving_offset;
      cur->func_data = number++;
      cur = cur->next_prog;
    }
  }

  return result;
}

rleprog_t*
compile_menu_text (const char *text, enum text_option topt,
		   int row, int col)
{
  return compile_rletext (menu_font, text, topt, 0, row * xbuf + col);
}
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

#include "common.h"
#include "rletext.h"
#include "const.h"
#include "debugmsg.h"

/*
 * Generate a RLE-program that display a text, using a given font.
 *
 * Currently, rleprog_t are computed for each character and chained
 * together.  This can be enhanced: unless waving, there is no point
 * in computing a REL-prog for each character, it may be better to
 * compute *one* REL-prog for the *whole* string.
 */

rleprog_t*
compile_rletext (const fontdata_t *font, const char *text, 
		 enum text_option topt, unsigned int maxwidth,
		 int offset)
{
  rleprog_t *result = 0;	/* RLE-program to return */
  rleprog_t **next = &result;	/* place for the next RLE-program */

  if (topt & T_FLUSHED_LEFT) {	/* FLUSHED_LEFT or JUSTIFIED */
    /* offset = 0; */
  } else {			/* FLUSHED_RIGHT or CENTERED */
    unsigned int text_width = compute_text_width (font, text, 0);

    if (topt & T_FLUSHED_RIGHT)	/* FLUSHED_RIGHT */
      offset -= (int) text_width;
    else			/* CENTERED */
      offset -= (int) text_width/2;
  }

  for (; *text; ++text) {
    if (*text == ' ') {
      offset += font->width[' '];
      /* FIXME: handle the JUSTIFIED case here. */
    } else {
      *next = compile_rleprog (font->upper_left[(int)*text], 0,
			       font->height, font->width[(int)*text],
			       font->line_size, xbuf);
      (*next)->dest_offset = offset;
      result->latest_known = *next;
      next = &((*next)->next_prog);
      offset += font->width[(int)*text];
    }
  }
  return result;
}

rleprog_t*
compile_menu_text (const char *text, enum text_option topt,
		   int row, int col)
{
  return compile_rletext (menu_font, text, topt, 0, row * xbuf + col);
}
