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
# $Id: Makefile.am 1.3 Sun, 02 Jul 2000 21:05:23 +0200 adl $

man_MANS = heroes.6
EXTRA_DIST = $(man_MANS)

if HAVE_HELP2MAN
heroes.6 : $(srcdir)/../src/argv.c
	@HELP2MAN@ -n game -s6 ../src/heroes | sed -e 's/ FSF//' > $@
endif
