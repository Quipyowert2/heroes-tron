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

#EXTRA_DIST = extralvl level rsc sprites txt_cfg

SUBDIRS = etc extralvl intro sprites rsc level txt_cfg hedit @musicdir@

bin_PROGRAMS = heroes

heroes_SOURCES = \
argv.c \
argv.h \
const.c \
const.h \
display.c \
display.h \
draw.c \
draw.h \
endian.h \
endscroll.c \
endscroll.h \
errors.c \
errors.h \
extras.c \
extras.h \
fastmem.h \
font.h \
font_help.h \
font_menu.h \
getshline.h \
getshline.c \
generic_list.h \
gfx_reader.h \
hedlite.c \
hedlite.h \
hendian.c \
hendian.h \
heroes.c \
heroes.h \
intro.c \
intro.h \
joystick.c \
joystick.h \
keyb.c \
keyb.h \
keys_heroes.h \
keysdef.h \
keysdef.c \
menus.c \
menus.h \
misc.c \
misc.h \
options.c \
options.h \
optionss.h \
pcx.c \
pcx.h \
pixelize.c \
pixelize.h \
render.c \
render.h \
savegame.c \
savegame.h \
scores.c \
scores.h \
sfx.c \
sfx.h \
sound.c \
sound.h \
start.h \
structs.h \
timer.c \
timer.h \
txts.c \
txts.h \
musicfiles.h \
musicfiles.c \
userconf.c \
userconf.h \
userdir.c \
userdir.h \
visuals.c \
visuals.h

BUILT_SOURCES = keysdef-inc.h
CLEANFILES = keysdef-inc.h

# keysdef-inc depends on config.h in order to be regenerated each time
# config.h change (in case the game is reconfigured with another display
# engine).
if GGI
keysdef-inc.h: config.h; @AWK@ '					     \
BEGIN { print "/* Do not edit!  This is generated automaticaly */\n"	     \
              "#ifndef __KEYSDEF_GII_H__\n"				     \
              "#define __KEYSDEF_GII_H__\n"				     \
              "#include \"keysdef.h\"\n"				     \
              "#include <ggi/keyboard.h>\n\n"				     \
              "const struct keynames_s keynames[] = {" }		     \
$$2 ~ /(GIIK_|GIIUC_)/ {						     \
	print "\t{ \""toupper(substr($$2,index($$2,"_")+1))"\",", $$2,"}," } \
END { print "\t{ 0, 0 }};\n#endif\n" }' < @GGI_KEYBOARD_H@ > $@
endif

if SDL
keysdef-inc.h: config.h; @AWK@ '					     \
BEGIN { print "/* Do not edit!  This is generated automaticaly */\n"	     \
              "#ifndef __KEYSDEF_SDL_H__\n"				     \
              "#define __KEYSDEF_SDL_H__\n"				     \
              "#include \"keysdef.h\"\n"				     \
              "#include <SDL/SDL_keysym.h>\n\n"				     \
              "const struct keynames_s keynames[] = {" }		     \
$$1 ~ /SDLK_/ {								     \
	print "\t{ \""toupper(substr($$1,index($$1,"_")+1))"\",", $$1,"}," } \
END { print "\t{ 0, 0 }};\n#endif\n" }' < @SDL_KEYSYM_H@ > $@
endif
