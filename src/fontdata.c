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
#include "const.h"
#include "debugmsg.h"
#include "fontdata.h"

fontdata_t *edit_font = 0;
fontdata_t *menu_font = 0;
fontdata_t *help_font = 0;
fontdata_t *deck_font = 0;
fontdata_t *bonus_font = 0;

static void
initialize_menu_font (void)
{
  pixel_t *upl;			/* upper left pixel of the character */
  int ch;			/* current character */

  XMALLOC_VAR (menu_font);
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

static void
initialize_deck_font (void)
{
  pixel_t *upl;			/* upper left pixel of the character */
  int ch;			/* current character */

  XMALLOC_VAR (deck_font);
  deck_font->height = 5;
  deck_font->line_size = font_deck_img.width;
  memset (deck_font->width, 0, 256);

  for (ch = ' '; ch <= '^'; ++ch) {
    unsigned int width, act_width;
    unsigned int height;

    upl = font_deck_img.buffer + 2 * font_deck_img.width +
      ((int) (ch - ' ') % 32) * 8 +
      ((int) (ch - ' ') / 32) * font_deck_img.width * 8;

    /* detect the width of a character */
    for (act_width = width = 0; width < 8; ++width)
      for (height = 0; height < deck_font->height; ++height) {
	if (upl[width + height * font_deck_img.width] != 0) {
	  act_width = width + 2;
	  break;
	}
      }
    deck_font->upper_left[ch] = upl;
    deck_font->width[ch] = act_width;
  }

  deck_font->width[' '] = 4;

  /* link the lower case characters to the upper */
  for (ch = 'a'; ch <= 'z'; ++ch) {
    deck_font->upper_left[ch] = deck_font->upper_left[ch - ('a' - 'A')];
    deck_font->width[ch] = deck_font->width[ch - ('a' - 'A')];
  }
}

static void
initialize_bonus_font (void)
{
  pixel_t *upl;			/* upper left pixel of the character */
  int ch;			/* current character */

  XMALLOC_VAR (bonus_font);
  bonus_font->height = 12;
  bonus_font->line_size = bonus_font_img.width;
  memset (bonus_font->width, 0, 256);

  for (ch = ' '; ch <= 'd'; ++ch) {
    unsigned int width, act_width;
    unsigned int height;

    upl = bonus_font_img.buffer +
      ((int) (ch - ' ') % 26) * 12 +
      ((int) (ch - ' ') / 26) * bonus_font_img.width * bonus_font->height;

    /* detect the width of a character */
    for (act_width = width = 0; width < 12; ++width)
      for (height = 0; height < bonus_font->height; ++height) {
	if (upl[width + height * bonus_font_img.width] != 0) {
	  act_width = width + 1;
	  break;
	}
      }
    bonus_font->upper_left[ch] = upl;
    bonus_font->width[ch] = act_width;
  }
  bonus_font->width[' '] = 5;
}

void
init_fonts (void)
{
  dmsg (D_MISC, "initializing font data");

  initialize_menu_font ();
  edit_font = 0;
  help_font = 0;
  initialize_deck_font ();
  initialize_bonus_font ();
}

void
uninit_fonts (void)
{
  dmsg (D_MISC, "uninitializing font data");

  XFREE0 (menu_font);
  XFREE0 (edit_font);
  XFREE0 (help_font);
  XFREE0 (deck_font);
  XFREE0 (bonus_font);
}

unsigned int
compute_text_width (const fontdata_t *font, const char *text,
		    int ignore_spaces)
{
  unsigned int width = 0;

  for (; *text; ++text)
    if (!ignore_spaces || *text != ' ')
      width += font->width[(int) *text];
  return width;
}
