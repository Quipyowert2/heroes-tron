/*------------------------------------------------------------------------.
| Copyright (C) 1997,1998,2000 Alexandre Duret-Lutz <duret_g@epita.fr>    |
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


#ifndef __EXTRAS_H__
#define __EXTRAS_H__

typedef	char*	filename_t;

typedef struct {
  char*		level_name;
  filename_t	full_name;
  char		is_in_user_dir;
} extra_level_t;

extern extra_level_t *extra_list;
extern char *extra_selected_list;
extern int extra_nbr;
extern int extra_user_nbr;

void browse_extra_directories (void);
void add_extra_directory (filename_t fn);
void add_default_extra_directories (void);
void free_extra_list (void);
void free_extra_directories (void);

#endif
