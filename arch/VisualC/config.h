/* arch/VisualC/config.h.  Generated manually from src/config.h.in.
                         VERSION is updated automatically by arch/Makefile. */
/* src/config.h.in. Generated automatically from configure.in by autoheader. */

/* Define if you have libggi. */
/* #undef HAVE_PKG_GGI */

/* Define if you have libgii. */
/* #undef HAVE_PKG_GII */

/* Define if you have libSDL_mixer. */
#define HAVE_PKG_sdl_mixer 1

/* Set to 1234 if the system is little-endian, 4321 if big-endian */
#define BYTEORDER 1234

/* Define if you want debuging code and symbols. */
/* #undef DEBUG */

/* Define to 1 if you have the declaration of `calloc', and to 0 if you don't.
   */
#define HAVE_DECL_CALLOC 1

/* Define to 1 if you have the declaration of `free', and to 0 if you don't.
   */
#define HAVE_DECL_FREE 1

/* Define to 1 if you have the declaration of `malloc', and to 0 if you don't.
   */
#define HAVE_DECL_MALLOC 1

/* Define to 1 if you have the declaration of `realloc', and to 0 if you
   don't. */
#define HAVE_DECL_REALLOC 1

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#define HAVE_DECL_STRERROR_R 0

/* Define if you have the <direct.h> header file. */
#define HAVE_DIRECT_H 1

/* Define if you have the <dirent.h> header file, and it defines `DIR'. */
/* #undef HAVE_DIRENT_H */

/* Define if you link with the dmalloc library. */
/* #undef HAVE_DMALLOC */

/* Define if the malloc check has been performed. */
#define HAVE_DONE_WORKING_MALLOC_CHECK 1

/* Define if the realloc check has been performed. */
#define HAVE_DONE_WORKING_REALLOC_CHECK 1

/* Define if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define if you have the <getopt.h> header file. */
/* #undef HAVE_GETOPT_H */

/* Define if you have the `getopt_long' function. */
/* #undef HAVE_GETOPT_LONG */

/* Define if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have the <io.h> header file. */
#define HAVE_IO_H 1

/* Define if you have the `dir' library (-ldir). */
/* #undef HAVE_LIBDIR */

/* Define if you have the `dmalloc' library (-ldmalloc). */
/* #undef HAVE_LIBDMALLOC */

/* Define if you have the `efence' library (-lefence). */
/* #undef HAVE_LIBEFENCE */

/* Define if you have the `ggi' library (-lggi). */
/* #undef HAVE_LIBGGI */

/* Define if you have the `gii' library (-lgii). */
/* #undef HAVE_LIBGII */

/* Define if you have the `gnugetopt' library (-lgnugetopt). */
/* #undef HAVE_LIBGNUGETOPT */

/* Define if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define if you have the LibMikMod library. */
/* #undef HAVE_LIBMIKMOD */

/* Define if you have the `SDL_mixer' library (-lSDL_mixer). */
#define HAVE_LIBSDL_MIXER 1

/* Define if you have the `x' library (-lx). */
/* #undef HAVE_LIBX */

/* Define if you have the `memcpy' function. */
#define HAVE_MEMCPY 1

/* Define if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define if you have the `mkdir' function. */
/* #undef HAVE_MKDIR */

/* Define if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define if you have libggi. */
/* #undef HAVE_PKG_GGI */

/* Define if you have libgii. */
/* #undef HAVE_PKG_GII */

/* Define if you have libSDL_mixer. */
#undef HAVE_PKG_sdl_mixer

/* Define if you have readdir and friends. */
/* #undef HAVE_READDIR */

/* Define if you have the SDL library. */
#define HAVE_SDL 1

/* Define if you have the `SDL_EnableKeyRepeat' function. */
#define HAVE_SDL_ENABLEKEYREPEAT 1

/* Define if you have the `SDL_JoystickOpen' function. */
#define HAVE_SDL_JOYSTICKOPEN 1

/* Define if you have the `sleep' function. */
/* #undef HAVE_SLEEP */

/* Define if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define if you have the <stdlib.h> header file. */
/* #undef HAVE_STDLIB_H */

/* Define if you have the `strcasecmp' function. */
/* #undef HAVE_STRCASECMP */

/* Define if you have the `strerror' function. */
/* #undef HAVE_STRERROR */

/* Define if you have the `strerror_r' function. */
/* #undef HAVE_STRERROR_R */

/* Define if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file, and it defines `DIR'. */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define if you have the <windows.h> header file. */
#define HAVE_WINDOWS_H 1

/* Define to 1 if `strerror_r' returns a string. */
/* #undef HAVE_WORKING_STRERROR_R */

/* Define if you have the `_mkdir' function. */
#define HAVE__MKDIR 1

/* The canonical host */
#define HOST "i386-pc-win32"

/* Define if you want joystick support. */
#define JOYSTICK_SUPPORT 1

/* Define if mkdir takes only one argument. */
#define MKDIR_TAKES_ONE_ARG 1

/* Name of package */
#define PACKAGE "heroes"

/* Define if compiler has function prototypes */
#define PROTOTYPES 1

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly. */
#define STAT_MACROS_BROKEN 1

/* Define if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* Define if you want tracing code. */
#define USE_HEROES_DEBUG 1

/* Version number of package */
#define VERSION "0.9a"

/* whether byteorder is bigendian */
/* #undef WORDS_BIGENDIAN */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Path to the data directory. */
#define datadir "../share/heroes"

/* Define to rpl_malloc if the replacement function should be used. */
#define malloc rpl_malloc

/* Define to rpl_realloc if the replacement function should be used. */
#define realloc rpl_malloc
