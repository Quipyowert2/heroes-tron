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
#include "rleprog.h"
#include "errors.h"

static void
unclipped_run (const rleprog_t* prog, pixel_t* dest)
{
  pixel_t*	cur;		/* current writting possition */
  u8_t*		pc;		/* program counter */
  u8_t*		epc;		/* end of program code */
    
  cur = dest + prog->dest_offset;
  pc = prog->code;
  epc = prog->end_code;
    
  while (pc < epc) {
    unsigned m, n;
    m = *pc++;
    n = *pc++;
    if (n == 0 && m == 0) {	/* end of line */
      cur += prog->line_skip;
    } else {
      cur += m;
      for (; n; --n)
	*cur++ = *pc++;		/* FIXME: would an inlined memcpy be better? */
    }
  }
}

void 
exec_rleprog (const rleprog_t* prog, pixel_t* dest)
{
  while (prog) {
    /* only unclipped run are currently supported */
    unclipped_run (prog, dest);
    prog = prog->next_prog;
  }
}


rleprog_t* 
compile_rleprog (const pixel_t* src, pixel_t transp_color,
		 unsigned int block_height, unsigned int block_width,
		 unsigned int src_width, unsigned int dest_width)
{
  rleprog_t* prog;
  unsigned int row;
  unsigned int code_size;
  u8_t* pc;			/* program counter */
  
  
  /* In the worst case (start with an opaque pixel and alternate
     transparant and opaque), we need three bytes to encode two
     pixels, plus three byte for the first pixel, and two for the end
     of line. */
  code_size = block_height * ((block_width / 2 + 1) * 3 + 2);

  if (!(prog = malloc (sizeof (*prog))) ||
      !(prog->code = malloc (code_size)))
    emsg ("Out of memory");
  
  /* encode the bloc */
  pc = prog->code;
  for (row = block_height; row; --row) {
    unsigned int m, n;
    const pixel_t* eol = src + block_width; /* end of line */
    
    /* encode a line */
    do {
      /* count the number of transparant pixels */
      for (m = 0; *src == transp_color && src < eol; ++src)
	++m;
      /* count the number of opaque pixels */
      for (n = 0; src[n] != transp_color && src + n < eol;)
	++n;
      
      /* write the corresponding program */
      *pc++ = m;
      *pc++ = n;
      for (; n; --n)
	*pc++ = *src++;
    }	while (src < eol);
    /* output an end of line */
    *pc++ = 0;
    *pc++ = 0;
      
    /* prepare for next line */
    src += src_width - block_width;
  }

  prog->end_code = pc;
  prog->line_skip = dest_width - block_width;
  prog->next_prog = 0;
  prog->dest_offset = 0;
  
  assert (pc < prog->code + code_size);

  return prog;
}

void free_rleprog (rleprog_t* prog)
{
  while (prog) {
    rleprog_t* next = prog->next_prog;
    free (prog);
    prog = next;
  }
}
