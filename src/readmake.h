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

#ifndef HEROES__READMAKE__H
#define HEROES__READMAKE__H

#include "sprite.h"


typedef struct private_read_data_s private_read_data_t;

typedef struct read_data_s read_data_t;
struct read_data_s {
  private_read_data_t *data;
  private_read_data_t *data_bg;
  int max;
};


/* Prepare STR for the reader rendering function, and a
   append it to the already cooked data in HEAD.  If HEAD is NULL,
   a new structure is allocated. */
read_data_t *compile_reader_data (read_data_t *head, const char *str);

/* Free any data allocated by compile_reader_data for RD. */
void free_reader_data (read_data_t *rd);

/* draw RD form line MIN to line MAX on DEST on DEST. */
void draw_reader_data (const read_data_t *rd, pixel_t *dest, int min, int max);

#endif /* HEROES__READMAKE__H */
