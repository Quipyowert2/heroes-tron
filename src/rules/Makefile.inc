## Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>
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

noinst_LIBRARIES += rules/libhrules.a

rules_libhrules_a_SOURCES= \
  rules/ai.c \
  rules/ai.h \
  rules/bonuses.c \
  rules/lemmings.c \
  rules/objects.c \
  rules/player.h \
  rules/player.c \
  rules/state.h \
  rules/state.c \
  rules/statepriv.h \
  rules/trail.c \
  rules/update.c
