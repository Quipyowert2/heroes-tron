AC_DEFUN([heroes_CHECK_DISPLAY_LIB_PRELIM],[
# handle choice between GGI and SDL

if test "${with_ggi-no}" != no; then
  if test "${with_sdl-no}" != no; then
    AC_MSG_ERROR([GGI and SDL can't be both used.])
  fi
  # disable SDL if GGI was selected
  with_sdl=no
else
  if test "${with_sdl-no}" != no; then
    # disable    GGI if SDL was selected
    with_ggi=no
  fi
fi])

dnl Heroes can run without display, but usually this is not what
dnl the user wants :), so this "feature" is enabled only if
dnl --disable-display has been given.
AC_DEFUN([heroes_CHECK_DISPLAY_LIB],
 [AC_ARG_ENABLE([display],
	        [AC_HELP_STRING([--disable-display],
                                [don't draw anything on screen])])
  display_lib='<disabled>'
  if test "${enable_display-yes}" = yes; then
    heroes_CHECK_SDL_PRELIM
    heroes_CHECK_DISPLAY_LIB_PRELIM
    heroes_CHECK_GGI([with_sdl=no; display_lib=LibGGI])
    heroes_CHECK_SDL([with_ggi=no; display_lib="SDL dnl
$sdl_config_major_version.dnl
$sdl_config_minor_version.dnl
$sdl_config_micro_version"])
    if test "$with_sdl:$with_ggi" = "no:no"; then
     AC_MSG_ERROR([No graphic library found.

Heroes requires either GGI or SDL to be installed on your system.
See the README file for pointers towards those libraries.
])
    fi
  else
    with_sdl=no
    with_ggi=no
  fi
  rm -f src/keysdef-inc.h
  heroes_CHECK_SDL_POST
  heroes_CHECK_GGI_POST
])
