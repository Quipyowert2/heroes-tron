# Copyright (C) 2000 Alexandre Duret-Lutz <duret_g@epita.fr> 
# 
# This file is part of Heroes.
# 
# Heroes is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your
# option) any later version.
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

EXTRA_DIST = strcasecmp.c

noinst_HEADERS= \
getopt.h \
getshline.h

# path for config.h
INCLUDES = -I../src

# These files are always compiled
libheroes_a_SOURCES= \
getopt.c \
getopt1.c \
getshline.c

# Additional files to compile, when functions are missing.
libheroes_a_LIBADD = @LIBOBJS@
libheroes_a_DEPENDENCIES = $(libheroes_a_LIBADD)
