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

#ifndef HEROES__RLEPROG__H
#define HEROES__RLEPROG__H

#include "display.h"

typedef struct rleprog_s rleprog_t;

struct rleprog_s {
  /* The code of an RLE-program is a sequence of
       1 u8_t: number m of transparent pixels to skip
       1 u8_t: number n of bytes to write
       n u8_t: actual bytes to write
     The end of a line can be announced using m=0 and n=0,
     and the code MUST terminate by m=0 and n=0.
  */
  u8_t*		code;

  /* a pointer the byte right after the end of code, 
     in order to known where to stop. */
  u8_t*		end_code;

  /* the number of byte to skip to from the end of a line to the
     beginning of the next one */
  unsigned int	line_skip;

  /* program can be chainned (use 0 otherwise) */
  rleprog_t*	next_prog;

  /* offset to add to the destination pointer given to exec_relprog.
     This is usefull because the same pointer is used for all
     RLE-program in a chain, but we don't wont to output all blocs on
     the same places. */
  unsigned int	dest_offset;

  /* latest program known in the chain, may not be the
     _actual_ latest (you still have to follow the next_prog pointer
     after this one); but if non null it can move you near the end
     of the chain rather quickly. */
  rleprog_t*	latest_known;
};

/* Execute an RLE-program, output the result to dest. */
void exec_rleprog (const rleprog_t* prog, pixel_t* dest);


/* Encode a bloc into an RLE-program */
rleprog_t* compile_rleprog (const pixel_t* src, pixel_t transp_color,
			    unsigned int block_height,
			    unsigned int block_width, 
			    unsigned int src_width, unsigned int dest_width);

/* Free an RLE-program, or a chain of. */
void free_rleprog (rleprog_t* prog);


/* concat two RLE-programs */
rleprog_t* concat_rleprog (rleprog_t* head, rleprog_t* tail);

#endif /* HEROES__RLEPROG__H */
