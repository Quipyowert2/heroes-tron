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


#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include "config.h"

#ifdef WORDS_BIGENDIAN

/* the files are stored in little endian, conversion routines
   needs only to exist when the host works in BIGENDIAN */

#ifdef linux

#include <asm/byteorder.h>
#define BSWAP16(x) __arch__swab16(x)
#define BSWAP32(x) __arch__swab32(x)

#else /* ! linux */

static __inline__ unsigned short int 
BSWAP16(unsigned short int x) 
{
  return (x<<8) | (x>>8);
}

static __inline__ unsigned long int
BSWAP32(unsigned long int x) 
{
  return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24);
}

#endif /* ! linux */

#else /* ! BIGENDIAN */

#define BSWAP16(x) (x)
#define BSWAP32(x) (x)

#endif /* ! BIGENDIAN */

#endif /* __ENDIAN_H__ */
