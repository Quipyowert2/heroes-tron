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
#include "const.h"
#include "debugmsg.h"
#include "fontdata.h"

fontdata_t *edit_font = 0;
fontdata_t *menu_font = 0;
fontdata_t *help_font = 0;
fontdata_t *deck_font = 0;

static void
initialize_menu_font (void)
{
  pixel_t *upl;			/* upper left pixel of the character */
  int ch;			/* current character */
    
  menu_font = malloc (sizeof (*menu_font));
  menu_font->height = 10;
  menu_font->line_size = main_font_img.width;
  memset (menu_font->width, 0, 256);

  for (ch = ' '; ch <= 'd'; ++ch) {
    unsigned int width, act_width;
    unsigned int height;
    
    upl = main_font_img.buffer + 
      ((int) (ch - ' ') % 14) * 22 + 
      ((int) (ch - ' ') / 14) * main_font_img.width * menu_font->height;
  
    /* detect the width of a character */
    for (act_width = width = 0; width < 22; ++width)
      for (height = 0; height < menu_font->height; ++height) {
	if (upl[width + height * main_font_img.width] != 0) {
	  act_width = width + 1;
	  break;
	}
      }
    menu_font->upper_left[ch] = upl;
    menu_font->width[ch] = act_width;
  }      
  menu_font->width[' '] = 5;
}

void
init_fonts (void)
{
  dmsg (D_MISC, "initializing font data");

  initialize_menu_font ();
  edit_font = 0;
  help_font = 0; 
  deck_font = 0;
}

void
uninit_fonts (void)
{
  dmsg (D_MISC, "uninitializing font data");

  if (menu_font) {
    free (menu_font);
    menu_font = 0;
  }
  if (edit_font) {
    free (edit_font);
    edit_font = 0;
  }
  if (help_font) {
    free (help_font);
    help_font = 0;
  }
  if (deck_font) {
    free (deck_font);
    deck_font = 0;
  }
}

unsigned int
compute_text_width (const fontdata_t *font, const char *text,
		    int ignore_spaces)
{
  unsigned int width = 0;
  
  for (; *text; ++text)
    if (!ignore_spaces || *text != ' ')
      width += font->width[*text];
  return width;
}
