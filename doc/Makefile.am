# Copyright 2000, 2001  Alexandre Duret-Lutz <duret_g@epita.fr>
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


info_TEXINFOS = heroes.texi
heroes_TEXINFOS = debugchn.texi people.texi
EXTRA_DIST = heroes.html

MAINTAINERCLEANFILES = debugchn.texi people.texi

if MAKE_HTML
hdir = @htmldir@
h_DATA = heroes.html
endif

SUFFIXES = .html

.texi.html:
	@cd $(srcdir) && rm -f $@ $@-[0-9] $@-[0-9][0-9]
	cd $(srcdir) && $(MAKEINFO) --html --no-split `echo $< | sed 's,.*/,,'`

debugchn.texi: $(top_srcdir)/src/debugchn.def $(top_srcdir)/src/debugchn.tpl
	$(AUTOGEN) -L $(top_srcdir)/src -o 'texi=$@' $(top_srcdir)/src/debugchn.def

people.texi: $(top_srcdir)/src/people.def $(top_srcdir)/src/people.tpl
	$(AUTOGEN) -L $(top_srcdir)/src -o 'texi=$@' $(top_srcdir)/src/people.def

CLEANFILES = heroes.cps heroes.vrs
