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

AC_DEFUN([heroes_CHECK_DISPLAY_LIB],
 [heroes_CHECK_SDL_PRELIM
  heroes_CHECK_DISPLAY_LIB_PRELIM
  display_lib='<disabled>'
  heroes_CHECK_GGI([with_sdl=no; display_lib=LibGGI])
  heroes_CHECK_SDL([with_ggi=no; display_lib="SDL dnl
$sdl_config_major_version.dnl
$sdl_config_minor_version.dnl
$sdl_config_micro_version"])
  if test "$with_sdl:$with_ggi" = "no:no"; then
   AC_MSG_ERROR([Heroes cannot compile without a graphic library.

Heroes requires either GGI or SDL to be installed on your system.
See the README file for pointers towards those libraries.
])
  fi])
