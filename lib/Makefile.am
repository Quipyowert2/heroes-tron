# Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>
#
# This file is part of Heroes.
#
# Heroes is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# Heroes is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

noinst_LIBRARIES= libheroes.a

CFLAGS = @CFLAGS@ @GGI_CFLAGS@ @GII_CFLAGS@ @LIBMIKMOD_CFLAGS@ @SDL_CFLAGS@ @SDL_MIXER_CFLAGS@

EXTRA_DIST= \
ftruncate.c \
getopt.c \
getopt1.c \
malloc.c \
memrchr.c \
realloc.c \
stpcpy.c \
strcasecmp.c \
strndup.c \
w_dirent.c

noinst_HEADERS= \
basename.h \
dirname.h \
error.h \
filelock.h \
fstrcmp.h \
gnugetopt.h \
getshline.h \
hash.h \
isdir.h \
stripslash.h \
w_dirent.h \
xalloc.h \
xstrduplwr.h

# path for config.h and system.h
INCLUDES = -I.. -I$(srcdir) -I../intl -I$(top_srcdir)/src

# These files are always compiled
libheroes_a_SOURCES= \
basename.c \
dirname.c \
error.c \
filelock.c \
fstrcmp.c \
getshline.c \
hash.c \
isdir.c \
stripslash.c \
xmalloc.c \
xstrdup.c \
xstrduplwr.c

# Additional files to compile, when functions are missing.
libheroes_a_LIBADD = @LIBOBJS@ @ALLOCA@
libheroes_a_DEPENDENCIES = $(libheroes_a_LIBADD)
