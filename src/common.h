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

/*
 * Every .c file SHOULD include this file as the FIRST.
 *
 * It defines macros needed for portability, and includes system
 * headers that may be needed by some parts of the game.  This means
 * that all .c files will therefore include all these system headers,
 * even if they don't need it; but since most of these headers can
 * have different names or may not be needed at all on some systems,
 * its safer if the selection is done in one unique place.  
 *
 * Since this file is always included first, local .h files can assume
 * that this file has already been included.
 *
 * Do NOT include local headers (except config.h), only system
 * headers are included here.
 */

#ifndef HEROES__COMMON__H
#define HEROES__COMMON__H

#include "config.h"

#include <stdio.h>
#include <sys/types.h>

#if STDC_HEADERS
# include <stdlib.h>
#endif

#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <errno.h>

#ifdef HAVE_STRING_H
# if (!defined STDC_HEADERS) && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# define dirent direct
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#if HAVE_DIRECT_H
# include <direct.h>
#endif

#if HAVE_IO_H
# include <io.h>
#endif

#if HAVE_VPRINTF || HAVE_DOPRNT
# ifdef STDC_HEADERS
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
# else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
# endif
#else
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_WINDOWS_H
# include <windows.h>
# ifndef HAVE_READDIR
   /* should be included after io.h */
#  include "w_dirent.h"
# endif
# ifndef HAVE_SLEEP
#  define sleep(x) (Sleep ((x) * 1000))
# endif
#endif

/* display-keyboard-mouse library */

#if defined HAVE_PKG_GGI && defined HAVE_SDL
# error "HAVE_PKG_GGI and HAVE_SDL can't be defined both"
#endif
#ifdef HAVE_PKG_GGI
# include <ggi/ggi.h>
#endif
#ifdef HAVE_SDL
# include <SDL.h>
#endif

/* joystick library */

#ifdef JOYSTICK_SUPPORT
# if defined HAVE_PKG_GII && defined HAVE_SDL_JOYSTICKOPEN
#  error "HAVE_PKG_GII and HAVE_SDL_JOYSTICKOPEN can't be defined both"
# endif
# ifdef HAVE_PKG_GII
#  include <ggi/gii.h>
# endif
# ifdef HAVE_SDL_JOYSTICKOPEN
#  include <SDL.h>
# endif
#endif

/* sound library */

#if defined HAVE_LIBMIKMOD && defined HAVE_LIBSDL_MIXER
# error "HAVE_LIBMIKMOD and HAVE_LIBSDL_MIXER can't be defined both"
#endif
#ifdef HAVE_LIBMIKMOD
# include <mikmod.h>
# include <pthread.h>
#endif
#ifdef HAVE_LIBSDL_MIXER
# ifndef HAVE_SDL
#  error "HAVE_LIBSDL_MIXER can't be defined if HAVE_SDL isn't"
# endif
# include <SDL_mixer.h>
#endif

/* common integer sizes */

#ifdef HAVE_STDINT_H
# include <stdint.h>
typedef uint32_t	u32_t;
typedef uint16_t	u16_t;
typedef uint8_t		u8_t;
typedef int32_t		s32_t;
typedef int16_t		s16_t;
typedef int8_t		s8_t;
#else
typedef unsigned int		u32_t;
typedef unsigned short int	u16_t;
typedef unsigned char		u8_t;
typedef signed int		s32_t;
typedef signed short int	s16_t;
typedef signed char		s8_t;
#endif

/* keep this header at the end of the include list, because it may
   define macro to change the declaration of malloc functions */
#ifdef HAVE_DMALLOC
# include <dmalloc.h>
#endif

/* miscellaneous prototypes for replacement functions */

#ifndef HAVE_STRCASECMP
int strcasecmp (const char *s1, const char *s2);
#endif

#ifdef HAVE_MKDIR
# ifdef MKDIR_TAKES_ONE_ARG
#  define mkdir(a,b) mkdir(a)
# endif
#else
# ifdef HAVE_WINDOWS_H
#  define mkdir(a,b) _mkdir(a)
# else
#  error "Don't know how to create a directory on this system."
# endif
#endif

/* Define S_ISDIR if it isn't already defined in sys/stat.h */
#if defined(S_IFDIR) && !defined(S_ISDIR)
# define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

/* Re-define S_ISDIR if the native implementation is broken. According to
   the autoconf info-file this is the case for Tektronix UTekV, Amdahl UTS
   and Motorola System V/88 */
#if defined(STAT_MACROS_BROKEN)
# undef S_ISDIR
# define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#endif /* HEROES__COMMON__H */
