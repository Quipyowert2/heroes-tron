# This Makefile is used only if you run GNU make.
# It includes `Makefile' so it usually make no difference, except
# that we can add special maintainer rules here and rely
# on GNU make features.

# Systems where /bin/sh is not the default shell need this.  The $(shell)
# command below won't work with e.g. stock DOS/Windows shells.
SHELL = /bin/sh

have-Makefile := $(shell test -f Makefile && echo yes)
have-configure := $(shell test -f configure && echo yes)

ifneq ($(have-Makefile),yes)
ifeq ($(have-configure),yes)
# If there is a ./configure but no Makefile, give the user a diagnostic.
all:
	@echo There seems to be no Makefile in this directory.
	@echo You must run ./configure before running make.
	@exit 1
else
# no configure? try to create it.
all: cvsboot

endif
AUTOGEN = autogen
# Those target are already in Makefile.am, but they are
# needed before Makefile.in can be generated.  So we define
# them here only if Makefile doesn't exist yet.
THANKS: src/people.def src/people.tpl
	$(AUTOGEN) -L src -o 'thanks=THANKS' $<

AUTHORS: src/people.def src/people.tpl
	$(AUTOGEN) -L src -o 'authors=AUTHORS' $<
else
include Makefile
endif

cvsboot: AUTHORS THANKS
	tools/cvsboot.sh

.PHONY: all cvsboot
# Tell version 3.79 and up of GNU make to not build goals in this
# directory in parallel.  This is necessary in case someone tries to
# build multiple targets on one command line.
.NOTPARALLEL:
