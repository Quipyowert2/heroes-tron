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
# $Id: Makefile.am 1.1 Sat, 13 May 2000 10:51:14 +0200 pollux $

xdir = @moddir@
x_DATA = $(subst $(srcdir)/,,$(wildcard $(srcdir)/*.xm)) sound.conf

EXTRA_DIST = $(subst $(srcdir)/,,$(wildcard $(srcdir)/*.xm)) sound.conf.in
CLEANFILES = sound.conf

sound.conf: sound.conf.in ../config.h
	sed -e "s:%%moddir%%:$(moddir):" < $(srcdir)/sound.conf.in > $@
