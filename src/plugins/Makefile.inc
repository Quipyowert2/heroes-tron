##                                                        -*- Makefile -*-
## Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>
##
## This file is part of Heroes.
##
## Heroes is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License version 2 as published by
## the Free Software Foundation.
##
## Heroes is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##

pkglib_LTLIBRARIES = plugins/stdai.la

plugins_stdai_la_SOURCES = plugins/stdai.c
plugins_stdai_la_LDFLAGS = -module
# plugins_stdai_la_CPPFLAGS would be ignored by Automake, so use _CFLAGS.
plugins_stdai_la_CFLAGS = $(MORE_WARNINGS_CFLAGS) -I./plugins

pkginclude_HEADERS = plugins/plugin.h
nodist_pkginclude_HEADERS = plugins/pconf.h
BUILT_SOURCES += plugins/plugin.h plugins/pconf.h

PLUGIN_H_INPUTS = \
  $(srcdir)/system.h \
  $(srcdir)/lvl/lvl.h \
  $(srcdir)/rules/player.h \
  $(srcdir)/rules/state.h \
  $(srcdir)/hooks.h \
  $(srcdir)/opponents.h

PUBHEX_AWK = $(top_srcdir)/tools/pubhex.awk
plugins/plugin.h: $(PUBHEX_AWK) $(PLUGIN_H_INPUTS)
	$(AWK) -f $(PUBHEX_AWK) $(PLUGIN_H_INPUTS) >plugins/plugin.h

EXTRA_SCRIPTS = plugins/extrconf.sh
PERL = perl
SCANDEFS_PL = $(top_srcdir)/tools/scandefs.pl
plugins/extrconf.sh: plugins/plugin.h $(top_srcdir)/lib/xalloc.h $(SCANDEFS_PL)
	$(PERL) $(SCANDEFS_PL) `test -f plugins/plugin.h || echo $(srcdir)/`plugins/plugin.h $(top_srcdir)/lib/xalloc.h $(SCANDEFS_PL) >plugins/extrconf.sh

plugins/pconf.h: plugins/extrconf.sh $(CONFIG_HEADER)
	$(SHELL) plugins/extrconf.sh $(CONFIG_HEADER) >plugins/pconf.h

DISTCLEANFILES += \
  plugins/pconf.h \
  plugins/extrconf.sh

# Workaround an Automake 1.6 bug.
distclean-local:
	rm -Rf plugins/.libs plugins/_libs
	rm -f plugins/plugins_stdai_la-*
