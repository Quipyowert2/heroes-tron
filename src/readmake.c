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
#include "readmake.h"
#include "parafmt.h"
#include "fontdata.h"
#include "const.h"
#include "sprtext.h"
#include "sprprog.h"
#include "sprrle.h"
#include "sprglenz.h"
#include "sprglauto.h"
#include "pcx.h"
#include "errors.h"

struct private_read_data_s {
  sprite_t *sprite;
  int line;
  private_read_data_t *next;
};

static read_data_t *
insert_read_data (read_data_t *rd, sprite_t *spr, int line, bool bg)
{
  private_read_data_t *p, *o, **q;
  NEW (private_read_data_t, res);

  if (!rd) {
    NEW (read_data_t, rhd);
    rhd->data_bg = 0;
    rhd->data = 0;
    rhd->max = 0;
    rd = rhd;
  }

  if (rd->max < line)
    rd->max = line;

  res->sprite = spr;
  res->line = line;

  if (bg)
    q = &rd->data_bg;
  else
    q = &rd->data;
  p = *q;

  if (!p || p->line >= line) {
    res->next = p;
    *q = res;
    return rd;
  }

  do {
    o = p;
    p = p->next;
  } while (p && p->line < line);
  o->next = res;
  res->next = p;
  return rd;
}

/* return the head of a string, cut on character ' ' (or '\0' or '}')
   and advance the input pointer to the character after that delimiter
   (unless '\0' is encountered in which case *STR is set to 0). */
static char *
readtok (char **src)
{
  int size;
  char *_src = *src;
  for (size = 0; _src[size] && _src[size] != ' ' && _src[size] != '}'; ++size)
    /* NOP */;
  if (_src[size]) {
    _src[size] = 0;
    *src = _src + size + 1;
  } else
    *src = 0;
  ++size;
  return _src;
}


/* default margins */
#define DEF_LM	5
#define DEF_RM	315

/* FIXME: get rid of this, use dynamically allocated buffers */
#define MAX_LINES	64

static void
compute_widths (width_t *wid, const int *lm, const int *rm, int glm)
{
  unsigned int line;
  /* compute widths */
  for (line = 0; line < MAX_LINES; ++line)
    wid[line] = rm[line] - lm[line] - glm;
  /* put a 0 where the widths start to be equal */
  for (line = MAX_LINES - 2; line > 1; --line)
    if (wid[line] != wid[line - 1])
      break;
  wid[line + 1] = 0;
}

static void
shift_margins (int *lm, int *rm)
{
  unsigned int line;
  /* shift the margin array */
  for (line = 0; line + 1 < MAX_LINES; ++line) {
    lm[line] = lm[line + 1];
    rm[line] = rm[line + 1];
  }
  lm[line] = DEF_LM;
  rm[line] = DEF_RM;
}

read_data_t *
compile_reader_data (read_data_t *head, const char *str)
{
  int lm[MAX_LINES];		/* left margin */
  int glm;			/* global left margin */
  int rm[MAX_LINES];		/* right margin */
  width_t wid[MAX_LINES];	/* width (= rm - lm) */
  char *curstr;
  char *curstr_allocated;
  int line;
  int voffset = 0;
  pcx_image_t help_pics_img;

  pcx_load_from_rsc ("help-pictures-img", &help_pics_img);

  /* initilialize lm and rm */
  for (line = 0; line < MAX_LINES; ++line) {
    lm[line] = DEF_LM;
    rm[line] = DEF_RM;
  }

  while (*str) {
    const char *start;
    enum text_option topt = T_JUSTIFIED;

    glm = 0;

    /* get a line */
    start = str;
    while (*str && *str != '\n')
      ++str;
    if (*str) {
      ++str;
      curstr = CCLONE (start, str - start);
      curstr[str - start - 1] = 0;
    } else
      curstr = xstrdup (start);
    curstr_allocated = curstr;

    /* scan for initial markup */
    while (curstr[0] == '%' && curstr[1] == '{') {
      char *cmd;
      curstr += 2;
      cmd = readtok (&curstr);
      if (!strcmp (cmd, "img")) {
	char *flags = readtok (&curstr);
	int y = atoi (readtok (&curstr));
	int x = atoi (readtok (&curstr));
	int h = atoi (readtok (&curstr));
	int w = atoi (readtok (&curstr));
	int cl = 0;
	bool bg = false;
	bool gl = false;
	bool redgl = false;
	int offset = 0;
	int i, j;
	for (;*flags ;++flags) {
	  switch (*flags) {
	  case 'B':
	    bg = true;
	    break;
	  case 'G':
	    gl = true;
	    break;
	  case 'r' :
	    redgl = true;
	    break;
	  case 'C':		/* centered */
	    offset = glm + lm[0] + (rm[0] - glm - lm[0] - w) / 2;
	    break;
	  case 'R':		/* right */
	    offset = rm[0] - w;
	    if (!bg)
	      for (j = 0, i = h; i > 0; i -= 10, ++j)
		rm[j] -= w;
	    break;
	  case 'L':		/* left */
	    offset = glm + lm[0];
	    if (!bg)
	      for (j = 0, i = h; i > 0; i -= 10, ++j)
		lm[j] += w;
	    break;
	  case 'V':
	    offset -= (h - 10) / 2 * xbuf;
	    break;
	  default:
	    emsg ("Unknown image flag '%c'.", *flags);
	  }
	}

	while (h>0) {
	  /* split the sprite in strip of 10 pixels heights */
	  sprite_t *s;
	  if (redgl) {
	    s = compile_sprglenz (IMGPOS (help_pics_img, y, x), 0,
				  glenz[6], 10, w, help_pics_img.width, xbuf);
	  } else {
	    s = (gl ? compile_sprglauto : compile_sprrle)
	      (IMGPOS (help_pics_img, y, x), 0,
	       10, w, help_pics_img.width, xbuf);
	  }
	  head = insert_read_data (head, s, offset + voffset + xbuf * cl * 10,
				   bg);
	  h -= 10;
	  y += 10;
	  ++cl;
	}
	/* FIXME: handle glenz images */
      } else if (!strcmp (cmd, "flushleft")) {
	topt = T_FLUSHED_LEFT;
      } else if (!strcmp (cmd, "flushright")) {
	topt = T_FLUSHED_RIGHT;
      } else if (!strcmp (cmd, "center")) {
	topt = T_CENTERED;
      } else if (!strcmp (cmd, "head")) {
	int color = atoi (readtok (&curstr));
	head = insert_read_data (head,
				 compile_sprtext_color (help_font, curstr,
							T_FLUSHED_LEFT
							| T_WAVING, 0, 25),
				 voffset,
				 false);
	head = insert_read_data
	  (head, compile_sprrle (IMGPOS (help_pics_img,
					 69 + color * 12, 0), 0,
				 10, 12, help_pics_img.width, xbuf),
	   voffset + 5, true);
	head = insert_read_data
	  (head, compile_sprglenz (IMGPOS (help_pics_img, 81, 12), 0,
				   glenz[((color - 1) ^ 1) + 2],
				   10, 310 - 12, help_pics_img.width, xbuf),
	   voffset + 5 + 12, true);
	voffset += xbuf * (help_font->line_skip + help_font->height);
	goto next_line;
      } else {
	if (cmd[0] == '>') {
	  glm += 5 * strlen (cmd);
	}
      }
    }

    /* no text to format? */
    if (!*curstr) {
      do {
	voffset += xbuf * (help_font->line_skip + help_font->height);
	shift_margins (lm, rm);
      } while (lm[0] != DEF_LM || rm[0] != DEF_RM);
      goto next_line;
    }

    /* format the paragraph */
    compute_widths (wid, lm, rm, glm);
    {
      char **p = parafmt_var (curstr, help_font->width, wid,
			      help_font->min_space_width);
      char **l = p;

      while (*l) {
	int offset;

	compute_widths (wid, lm, rm, glm);

	if (!l[1] && (topt & T_JUSTIFIED) == T_JUSTIFIED)
	  topt = T_FLUSHED_LEFT;

	switch (topt) {
	case T_FLUSHED_LEFT:
	case T_JUSTIFIED:
	  offset = glm + lm[0];
	  break;
	case T_FLUSHED_RIGHT:
	  offset = rm[0];
	  break;
	case T_CENTERED:
	  offset = glm + lm[0] + (rm[0] - glm - lm[0]) / 2;
	  break;
	default:
	  assert (0);
	}

	head = insert_read_data (head,
				 compile_sprtext_color (help_font, *l,
							topt, *wid, offset),
				 voffset, false);
	voffset += xbuf * (help_font->line_skip + help_font->height);
	++l;
	shift_margins (lm, rm);
      }
      free_pararray (p);

      while (lm[0] != DEF_LM || rm[0] != DEF_RM) {
	voffset += xbuf * (help_font->line_skip + help_font->height);
	shift_margins (lm, rm);
      }
    }
  next_line:
    free (curstr_allocated);
  }
  img_free (&help_pics_img);
  return head;
}

void
free_reader_data (read_data_t *rd)
{
  private_read_data_t *p = rd->data;
  while (p) {
    private_read_data_t *n = p->next;
    free_sprite (p->sprite);
    free (p);
    p = n;
  }
  p = rd->data_bg;
  while (p) {
    private_read_data_t *n = p->next;
    free_sprite (p->sprite);
    free (p);
    p = n;
  }
  free (rd);
}

void
draw_reader_data (const read_data_t *rd, pixel_t *dest, int min, int max)
{
  const private_read_data_t *p;
  min *= xbuf;
  max *= xbuf;
  /* draw the background */
  p = rd->data_bg;
  while (p && p->line < min)
    p = p->next;
  while (p && p->line <= max) {
    DRAW_SPRITE (p->sprite, dest + p->line - min);
    p = p->next;
  }
  /* draw the foreground */
  p = rd->data;
  while (p && p->line < min)
    p = p->next;
  while (p && p->line <= max) {
    DRAW_SPRITE (p->sprite, dest + p->line - min);
    p = p->next;
  }

}
