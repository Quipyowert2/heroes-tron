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

#ifndef HEROES__SYSTEM__H
#define HEROES__SYSTEM__H

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

#if HAVE_STRING_H
# if HAVE_MEMORY_H && ! STDC_HEADERS
#  include <memory.h>
# endif
# include <string.h>
#else
# if HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_DIRENT_H
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
# if STDC_HEADERS
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

#if HAVE_GETOPT_H && HAVE_GETOPT_LONG
# include <getopt.h>
#else
# include "getopt.h"
#endif

/* Take care of NLS matters.  */

#if HAVE_LOCALE_H
# include <locale.h>
#endif
#if !HAVE_SETLOCALE
# define setlocale(Category, Locale) /* empty */
#endif

#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) Text
#endif
#define N_(Text) Text

#if HAVE_WINDOWS_H
# include <windows.h>
# if ! HAVE_READDIR
   /* should be included after io.h */
#  include "w_dirent.h"
# endif
# if ! HAVE_SLEEP
#  define sleep(x) (Sleep ((x) * 1000))
# endif
#endif

/* display-keyboard-mouse library */

#if HAVE_LIBGGI && HAVE_LIBSDL
# error "HAVE_LIBGGI and HAVE_LIBSDL can't be defined both"
#endif

#if HAVE_LIBGGI
# include <ggi/ggi.h>
typedef uint32		keycode_t;
#else /* !HAVE_LIBGGI */
# if HAVE_LIBSDL
#  include <SDL.h>
typedef SDLKey		keycode_t;
# else /* !HAVE_LIBSDL */
typedef unsigned int	keycode_t;
# endif /* !HAVE_LIBSDL */
#endif /* !HAVE_LIBGGI */
/* joystick library */

#if JOYSTICK_SUPPORT
# if HAVE_LIBGII && HAVE_SDL_JOYSTICKOPEN
#  error "HAVE_LIBGII and HAVE_SDL_JOYSTICKOPEN can't be defined both"
# endif
# if HAVE_LIBGII
#  include <ggi/gii.h>
# endif
# if HAVE_SDL_JOYSTICKOPEN && ! HAVE_LIBSDL
#  error "HAVE_SDL_JOSTICKOPEN can't be defined if HAVE_LIBSDL isn't"
/* Hence SDL.h is already included when HAVE_SDL_JOSTICKOPEN. */
# endif
#endif

/* sound library */

#if HAVE_LIBMIKMOD && HAVE_LIBSDL_MIXER
# error "HAVE_LIBMIKMOD and HAVE_LIBSDL_MIXER can't be defined both"
#endif
#if HAVE_LIBMIKMOD
# include <mikmod.h>
# include <pthread.h>
#endif
#if HAVE_LIBSDL_MIXER
# if ! HAVE_LIBSDL
#  error "HAVE_LIBSDL_MIXER can't be defined if HAVE_LIBSDL isn't"
# endif
# include <SDL_mixer.h>
#endif

/* boolean type */

#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
typedef enum {false = 0, true = 1} bool;
#endif

/* common integer sizes */

#if HAVE_STDINT_H
# include <stdint.h>
typedef uint32_t	u32_t;
typedef uint16_t	u16_t;
typedef uint8_t		u8_t;
typedef int32_t		s32_t;
typedef int16_t		s16_t;
typedef int8_t		s8_t;
#define U32_MAX		UINT32_MAX
#else
typedef unsigned int		u32_t;
typedef unsigned short int	u16_t;
typedef unsigned char		u8_t;
typedef signed int		s32_t;
typedef signed short int	s16_t;
typedef signed char		s8_t;
#define U32_MAX		(0xffffffffU)
#endif
#define UCHAR(c) ((unsigned char) (c))

/* xalloc.h (from fileutils.h) define __attribute__ only for gcc >= 2.8
   (according to the ChangeLog this is needed for OPENStep 4.2 cc)
   while we want to allow __attribute__ for gcc >= 2.7 in Heroes
   (because 2.7.2 is still widely used and __attribute__((packed)) is
   mandatory).  So we need to undefine __attribute__ after xalloc has been
   included. */
#ifndef __attribute__
/* remember that __attribute has to be undefined after inclusion */
# define undefine__attribute__
#endif
#include "xalloc.h"
#ifdef undefine__attribute__
# undef __attribute__
# undef undefine__attribute__
#endif

/* Like XFREE, but also zeroes Var */
#define XFREE0(Var)				\
  do {						\
    if (Var) {					\
      free (Var);				\
      (Var) = 0;				\
    }						\
  } while (0)

#define XCALLOC_ARRAY(Array, N_items) \
Array = xcalloc (sizeof (*(Array)), (N_items))

#define XMALLOC_ARRAY(Array, N_items) \
Array = xmalloc (sizeof (*(Array)) * (N_items))

#define XREALLOC_ARRAY(Array, N_items) \
Array = xrealloc ((Array), sizeof (*(Array)) * (N_items))

#define XMALLOC_VAR(Var) XMALLOC_ARRAY ((Var), 1)
#define XCALLOC_VAR(Var) XCALLOC_ARRAY ((Var), 1)

/* Like XMALLOC_ARRAY but also performs a memSet */
#define XSALLOC_ARRAY(Array, N_items, Val)			\
  do {								\
    XMALLOC_ARRAY ((Array), (N_items));				\
    memset ((Array), (Val), sizeof (*(Array)) * (N_items));	\
  } while (0)

#if HAVE_MTRACE
# include <mcheck.h>
#else
# define mtrace()
# define muntrace()
#endif

/* keep this header at the end of the include list, because it may
   define some macros to change the declaration of malloc functions */
#if HAVE_DMALLOC_H
# define DMALLOC_FUNC_CHECK
# include <dmalloc.h>
#endif
#if HAVE_MPATROL_H
# include <mpatrol.h>
#endif

/* miscellaneous prototypes for replacement functions */

#if ! HAVE_STRCASECMP
int strcasecmp (const char *s1, const char *s2);
#endif

#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
#  define mkdir(a,b) mkdir(a)
# endif
#else
# if HAVE__MKDIR
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

/* ===== The following ctype definition are stolen from textutils ===== */

/* Jim Meyering writes:

   "... Some ctype macros are valid only for character codes that
   isascii says are ASCII (SGI's IRIX-4.0.5 is one such system --when
   using /bin/cc or gcc but without giving an ansi option).  So, all
   ctype uses should be through macros like ISPRINT...  If
   STDC_HEADERS is defined, then autoconf has verified that the ctype
   macros don't need to be guarded with references to isascii. ...
   Defining isascii to 1 should let any compiler worth its salt
   eliminate the && through constant folding."

   Bruno Haible adds:

   "... Furthermore, isupper(c) etc. have an undefined result if c is
   outside the range -1 <= c <= 255. One is tempted to write isupper(c)
   with c being of type `char', but this is wrong if c is an 8-bit
   character >= 128 which gets sign-extended to a negative value.
   The macro ISUPPER protects against this as well."  */

#if STDC_HEADERS || (!defined (isascii) && !HAVE_ISASCII)
# define IN_CTYPE_DOMAIN(c) 1
#else
# define IN_CTYPE_DOMAIN(c) isascii(c)
#endif

#ifdef isblank
# define ISBLANK(c) (IN_CTYPE_DOMAIN (c) && isblank (c))
#else
# define ISBLANK(c) ((c) == ' ' || (c) == '\t')
#endif
#ifdef isgraph
# define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isgraph (c))
#else
# define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isprint (c) && !isspace (c))
#endif

/* This is defined in <sys/euc.h> on at least Solaris2.6 systems.  */
#undef ISPRINT

#define ISPRINT(c) (IN_CTYPE_DOMAIN (c) && isprint (c))
#define ISALNUM(c) (IN_CTYPE_DOMAIN (c) && isalnum (c))
#define ISALPHA(c) (IN_CTYPE_DOMAIN (c) && isalpha (c))
#define ISCNTRL(c) (IN_CTYPE_DOMAIN (c) && iscntrl (c))
#define ISLOWER(c) (IN_CTYPE_DOMAIN (c) && islower (c))
#define ISPUNCT(c) (IN_CTYPE_DOMAIN (c) && ispunct (c))
#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
#define ISUPPER(c) (IN_CTYPE_DOMAIN (c) && isupper (c))
#define ISXDIGIT(c) (IN_CTYPE_DOMAIN (c) && isxdigit (c))
#define ISDIGIT_LOCALE(c) (IN_CTYPE_DOMAIN (c) && isdigit (c))

#if STDC_HEADERS
# define TOLOWER(Ch) tolower (Ch)
# define TOUPPER(Ch) toupper (Ch)
#else
# define TOLOWER(Ch) (ISUPPER (Ch) ? tolower (Ch) : (Ch))
# define TOUPPER(Ch) (ISLOWER (Ch) ? toupper (Ch) : (Ch))
#endif

/* ISDIGIT differs from ISDIGIT_LOCALE, as follows:
   - Its arg may be any int or unsigned int; it need not be an unsigned char.
   - It's guaranteed to evaluate its argument exactly once.
   - It's typically faster.
   Posix 1003.2-1992 section 2.5.2.1 page 50 lines 1556-1558 says that
   only '0' through '9' are digits.  Prefer ISDIGIT to ISDIGIT_LOCALE unless
   it's important to use the locale's definition of `digit' even when the
   host does not conform to Posix.  */
#define ISDIGIT(c) ((unsigned) (c) - '0' <= 9)

/* ===== end of ctype definitions ===== */

#ifndef __attribute__
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7) || __STRICT_ANSI__
#  define __attribute__(x)
# endif
#endif

#ifndef ATTRIBUTE_NORETURN
# define ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#endif

#ifndef ATTRIBUTE_UNUSED
# define ATTRIBUTE_UNUSED __attribute__ ((unused))
#endif

#ifndef ATTRIBUTE_PACKED
# define ATTRIBUTE_PACKED __attribute__ ((packed))
#endif

#ifndef ATTRIBUTE_PRINTF
# define ATTRIBUTE_PRINTF(str,fst) __attribute__ ((format (printf, str, fst)))
#endif


#ifndef PTR_TO_INT
# define PTR_TO_INT(P) (((char *) P) - (char *) 0)
#endif
#ifndef INT_TO_PTR
# define INT_TO_PTR(P, TYPE) ((P) + ((TYPE) *) 0)
#endif

#endif /* HEROES__SYSTEM__H */
