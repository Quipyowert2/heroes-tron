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

EXTRA_DIST = \
VisualC/README.html \
VisualC/README.txt \
VisualC/VisualC.zip

# The files for VisualC are distributed compressed but
# we need to uncompress them to update VisualC/config.h
# when the version change.

VisualC_files = \
VisualC/config.h \
VisualC/keysdef-inc.h \
VisualC/heroes.dsp \
VisualC/heroes.dsw

VisualC/VisualC.zip: $(VisualC_files)
	$(ZIP) -9 $@ $(VisualC_files)

VisualC/config.cp:
	if test -f VisualC/VisualC.zip; then \
	  $(UNZIP) -o VisualC/VisualC.zip; \
	fi
	cp VisualC/config.h VisualC/config.cp

VisualC/config.h: VisualC/config.cp Makefile
	sed -e 's/^.*#.*def.*VERSION.*$$/#define VERSION "@VERSION@"/' < \
	  $< > $@

CLEANFILES = VisualC/config.cp

# Overwrite the distclean rule to ensure VisualC/VisualC.zip is built before
# erasing the files used to build it !
distclean-local: VisualC/VisualC.zip
	rm -f $(VisualC_files) $(CLEANFILES)
