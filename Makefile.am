# Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>
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

# dist-bzip2 is new in automake 1.4a and will therefore generate an
# error in 1.4.  Since it's bugged in 1.4a anyway (produce heroes-x.x.bz2
# instead of heroes-x.x.tar.bz2) we keep the 1.4 compatibility until it's
# fixed.
AUTOMAKE_OPTIONS = 1.4 gnits # dist-bzip2
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = ANNOUNCE BUGS HACKING \
misc/heroes-1.xpm \
misc/heroes-2.xpm \
misc/heroes-3.xpm \
misc/heroes-4.xpm \
misc/heroes-5.xpm \
misc/heroes-6.xpm \
misc/heroes-7.xpm \
misc/heroes-8.xpm

SUBDIRS = arch tools lib intl src m4 man po doc

# Remove this file here (it is created via configure), not from within intl.
DISTCLEANFILES = intl/libintl.h

README: $(top_srcdir)/doc/heroes.texi $(top_srcdir)/tools/readme.sed
	LC_ALL=C $(MAKEINFO) --no-header -I $(top_srcdir)/doc heroes.texi | \
	sed -n -f $(top_srcdir)/tools/readme.sed > README.tmp
	mv README.tmp README
