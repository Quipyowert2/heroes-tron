
/*------------------------------------------------------------------------.
| Copyright 2000, 2001  Alexandre Duret-Lutz <duret_g@epita.fr>           |
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

#ifndef HEROES__VIDEO_LOW__H
# define HEROES__VIDEO_LOW__H
# include "video.h"

/* Low-level video functions, only called from the media/video.c.  */

typedef void (*copy_function_t)(const pixel_t *source,
				pixel_t *dest_t,
				unsigned width);

/* Return true on success.  */
void init_video_low (int stretch, int *pitch);
void uninit_video_low (void);
void vsynchro_low (const pixel_t *s, copy_function_t f);
void vsynchro2_low (const pixel_t *s1, const pixel_t *s2, copy_function_t f);

#endif /* HEROES__VIDEO_LOW__H */
