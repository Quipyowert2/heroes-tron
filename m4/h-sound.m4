AC_DEFUN([heroes_CHECK_SOUND_LIB_PRELIM],
[# SDL_mixer imply SDL
if test "${with_sdl_mixer-no}" != no; then
  if test "${with_ggi-no}" != no; then
    AC_MSG_ERROR([GGI and SDL_mixer can't be both used.])
  fi
  with_ggi=no
fi

# handle choice between SDL_mixer and libMikmod
if test "${with_mikmod-no}" != no; then
  if test "${with_sdl_mixer-no}" != no; then
     AC_MSG_ERROR([libMikMod and SDL_mixer can't be both used.])
  fi
  # disable SDL_mixer if libMikMod was selected
  with_sdl_mixer=no
else
  if test "${with_sdl_mixer-no}" != no; then
    # disable libMikMod if SDL_mixer was selected
    with_mikmod=no
  fi
fi])

AC_DEFUN([heroes_CHECK_SOUND_LIB],
[AC_ARG_ENABLE([sound],
               [AC_HELP_STRING([--disable-sound],[turn off sound support])])
 if test "${enable_sound-yes}" != no; then
   heroes_CHECK_SDL_MIXER([sound_lib="SDL_mixer"; with_mikmod=no],
	                  [heroes_CHECK_MIKMOD([sound_lib="LibMikMod dnl
$libmikmod_config_major_version.dnl
$libmikmod_config_minor_version.dnl
$libmikmod_config_micro_version"],[sound_lib="<disabled>"])])
 fi])
