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

EXTRA_DIST = ABOUT-NLS ANNOUNCE BUGS HACKING \
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

# additional checks to perform during distcheck.
# this is heavily inspired from Jim Meyring's setup
# in Fileutils & Shellutils & Textutils.

distcheck-hook: man-distcheck my-distcheck-ggi my-distcheck-sdl

# Use this to make sure we don't run these programs when building
# from a virgin tgz file, below.
null_AM_MAKEFLAGS = \
  ACLOCAL=false \
  AUTOCONF=false \
  AUTOMAKE=false \
  AUTOHEADER=false \
  MAKEINFO=false

t=./=test
# build with libggi and mikmod (VPATH build)
my-distcheck-ggi:
	-rm -rf $(t)
	mkdir $(t)
	cd $(t) \
	  && ../$(distdir)/configure --disable-nls --with-ggi --with-gii \
			             --with-mikmod \
	  && $(MAKE) AM_MAKEFLAGS='$(null_AM_MAKEFLAGS)'
	-rm -rf $(t)

# build with libsdl and sdl_mixer (build in current directory, then
# ensure that no files were modified).
my-distcheck-sdl:
	-rm -rf $(t)
	mkdir $(t)
	GZIP=$(GZIP) $(AMTAR) -C $(t) -zxf $(distdir).tar.gz
	cd $(t)/$(distdir) \
	  && ./configure --disable-nls --with-sdl --with-sdl-mixer \
			 --enable-html-doc \
	  && $(MAKE) AM_MAKEFLAGS='$(null_AM_MAKEFLAGS)' \
	  && $(MAKE) dvi \
	  && $(MAKE) check \
	  && $(MAKE) distclean \
	  && rm -rf $(DEPDIR)
# Removing $(DEPDIR) like this is a gross kludge to work around a bug
# in automake.  Remove that line once it's fixed.
	cd $(t) && mv $(distdir) $(distdir).new \
	  && $(AMTAR) -zxf ../$(distdir).tar.gz
	diff -ur $(t)/$(distdir) $(t)/$(distdir).new
	-rm -rf $(t)

man-distcheck:
	cd man && $(MAKE) man-distcheck
