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

/*-------------------------------------------------------------------.
| This is a "smart" paragraph formating function.  The algorithm     |
| used here is roughly the same as in the command `fmt' from the GNU |
| fileutils except that here we have to handle spaces of variable    |
| width and letters does not all have the same width.                |
`-------------------------------------------------------------------*/

#include "system.h"
#include "parafmt.h"

typedef struct word_s		word_t;
typedef struct paragraph_s	paragraph_t;
typedef u32_t			cost_t;
#define MAX_COST		U32_MAX
#define SQR(x) ((x) * (x))

struct word_s {
  char		*letters;	/* positions in the string copy */
  char		*letters_end;	/* end of the word in the string copy */
  width_t	width;		/* width of the word */
  unsigned int	spaces;		/* number of spaces after the word */
  bool		is_punct;	/* if the word it is ended by a
				   punctuation mark */
  const word_t	*next_break;	/* Next word to break after, if
				   this one is broken after. */
  cost_t	next_break_cost; /* Cost for this next break. */
};

struct paragraph_s {
  word_t	*words;		/* The list of words for the paragraph. */
  unsigned int	nwords;		/* The number of words. */
  unsigned int	indent;		/* Initial spaces, before the first word. */
  width_t	max_width;	/* maximum width of the paragraph */
  width_t	std_space_width; /* standard space width */
  width_t	min_space_width; /* minimun space_width */
  const word_t	*first_break;	/* first word to break after */
  char		*data;		/* a copy of the original string */
};

/* count the number of words in STR.
   Words are sequences of non-space characters */
static unsigned int
count_words (const char *str)
{
  unsigned int count = 0;

  for (;;) {
    /* skip initial spaces */
    while (*str && ISSPACE (*str))
      ++str;
    /* at end of string? */
    if (!*str)
      break;
    ++count;
    /* skip the word */
    while (*str && !ISSPACE (*str))
      ++str;
  }
  return count;
}

/* Split STR in an array of words.  Return this as a paragraph_t. */
static paragraph_t *
split_words (const char *input_str, width_t *wa)
{
  unsigned int nwords;
  word_t *words;
  unsigned int i;
  char *str;
  NEW (paragraph_t, p);

  /* count initial spaces */
  for (i = 0; *input_str && ISSPACE (*input_str); ++input_str)
    ++i;
  p->indent = i;

  /* duplicate the string, without the leading spaces */
  p->data = str = xstrdup (input_str);

  /* allocate the words array */
  nwords = count_words (str);
  XMALLOC_ARRAY (words, nwords);
  p->nwords = nwords;
  p->words = words;

  /* fill the words array */
  for (i = 0; i < nwords; ++i) {
    words[i].letters = str;

    /* compute the width_t of the word
       (previous spaces have been skipped, and *str!=0 because i<nwords) */
    {
      width_t w = 0;
      do {
	w += wa[UCHAR (*str)];
	++str;
      } while (*str && !ISSPACE (*str));
      words[i].width = w;
    }

    /* record the end */
    words[i].letters_end = str;

    /* mark the word if it is ended by punctuatuion.
       The do/while above has run at least once so str[-1] is ok. */
    words[i].is_punct = !!ISPUNCT (str[-1]);

    /* count the spaces following current word */
    {
      int j;
      for (j = 0; *str && ISSPACE (*str); ++str)
	++j;
      words[i].spaces = j;
    }
  }

  return p;
}

/* Give the cost for breaking after word WN on a line
   which is WIDTH large and has SPACES spaces.
   You should play with the formulas here to give
   a better looking to the paragraph formating. */
static cost_t
compute_break_cost (paragraph_t *p, unsigned int wn,
		    width_t width, width_t spaces)
{
  cost_t cost = 0;

  if (spaces) {
    width_t space_width, std_space_width;

    /* mean width for spaces, manifolded for accuracy */
    space_width = (p->max_width - width) * 4 / spaces;
    space_width = (space_width * 4) / spaces;

    /* prefered width for spaces, also manifolded for comparision */
    std_space_width = p->std_space_width * 4;

    /* prefer the standard space width */
    cost += SQR (space_width - std_space_width);
  } else {
    /* isolated word: high penalty */
    cost += 1000000;
  }

  /* prefer breaking on punctuation */
  if (!p->words[wn].is_punct)
    cost += 50;

  /* prefer to use as much characters as possible on the line */
  cost += SQR (p->max_width - width) * 4;

  /* account for the cost of following breaks */
  cost += p->words[wn].next_break_cost;

  return cost;
}

/* Find the best break for a line starting on word wn.  Update
   res->next_break and res->next_break_cost accordingly.
   WARNING: this does not handle words larger than p->max_width */
static const word_t *
compute_best_break (paragraph_t *p, unsigned int wn, word_t *res)
{
  unsigned int nwords = p->nwords;
  const word_t *w = p->words;
  unsigned int spaces = 0;
  width_t width = 0;

  cost_t best_cost = MAX_COST;
  const word_t *best_break = 0;

  /* find the best break position on this line */
  while (wn < nwords) {
    cost_t cost;

    /* Add a new word */
    width += w[wn].width;

    /* don't check more words than what can fit on this line */
    if ((width + spaces * p->min_space_width) > p->max_width)
      break;

    /* consider breaking after this word */
    cost = compute_break_cost (p, wn, width, spaces);
    if (cost < best_cost) {
      best_cost = cost;
      best_break = w + wn;
    }

    /* advance to next word */
    spaces += w[wn].spaces;
    ++wn;
  }

  /* update res accordingly */
  if (res) {
    res->next_break = best_break;
    res->next_break_cost = (wn >= nwords) ? 0 : best_cost;
  }
  return best_break;
}

/* compute breaking paths for the paragraph P */
static void
compute_breaking_path (paragraph_t *p)
{
  word_t *w = p->words;
  unsigned int wn;

  for (wn = p->nwords; wn > 0; --wn) {
    /* consider breaking right before w[wn] */
    compute_best_break (p, wn, w + wn - 1);
  }

  /* finally, compute the best break for the first line */
  /* FIXME: p->indent should be dealed with somewhere*/
  p->first_break = compute_best_break (p, 0, 0);
}

/* follow the (already computed) breaking path,
   and build and array of strings (one per line) */
static char **
convert_paragraph_to_array (paragraph_t *p)
{
  int nlines;
  const word_t *fw;		/* first word of the current line */
  const word_t *lw;		/* last word of the current line */
  char **result;
  int curline;

  /* count the number of lines */
  for (lw = p->first_break, nlines = 0; lw; lw = lw->next_break)
    ++nlines;

  XMALLOC_ARRAY (result, nlines + 1);
  result[nlines] = 0;		/* mark the end of the array */

  fw = p->words;
  lw = p->first_break;
  for (curline = 0; curline < nlines; ++curline) {
    /* copy the words for the current line */
    if (lw)
      *lw->letters_end = 0;	/* split the string */
    result[curline] = fw->letters;
    /* jump to next line */
    fw = lw + 1;
    lw = lw->next_break;
  }
  return result;
}

char **
parafmt (const char *str, const width_t *wa,
	 width_t max_width, width_t min_space_width)
{
  char **result;
  paragraph_t *p = split_words (str, wa);
  /* FIXME: make sure not words are larger than max_width or
     compute_breaking_path may fail. */
  p->max_width = max_width;
  p->min_space_width = min_space_width;
  p->std_space_width = wa[' '];
  compute_breaking_path (p);
  result = convert_paragraph_to_array (p);
  free (p->words);
  free (p);
  return result;
}

void
free_pararray (char **p)
{
  free (p[0]);			/* this free ALL lines (p[0] is
				   the ->data member of the original
				   paragraph). */
  free (p);
}

#ifdef TEST
/*-----------------------------------------------------------------.
| Run "make parafmt-check" in the src/ directory to compile this.  |
`-----------------------------------------------------------------*/

static char **
print_array (char **array, width_t width)
{
  char spec[30];
  char **a = array;
  sprintf (spec, "%%-%us|\n", width);
  for (; *a; ++a)
    printf (spec, *a);
  return array;
}

static void
check_parafmt (width_t width)
{
  unsigned int test_widths[256];
  const char *test_str = "\
This is a quit longish string, used to test whether parafmt format \
it correctly.  Blah blah, blah blah.  You can check that break on \
punctiation (non alaphanumeric, non spaces) are sometime prefered.";
  int i;
  for (i = 0; i < 256; ++i)
    test_widths[i] = 1;
  free_pararray (print_array (parafmt (test_str, test_widths, width, 1), width));
}

char *program_name;

int
main (void)
{
  int i;
  for (i = 20; i < 80; ++i)
    check_parafmt (i);
  return 0;
}
#endif
