AC_DEFUN([heroes_CHECK_SDL_MIXER],
[# don't even try to check for SDL_mixer if SDL is not installed
if test "${with_sdl-yes}" = no; then
  with_sdl_mixer=no
fi # with_sdl

if test "${with_sdl_mixer-yes}" != no; then
  # Provide a config.h help string, as AC_caolan_CHECK_PACKAGE does not.
  AH_TEMPLATE([HAVE_PKG_SDL_MIXER],[Define if you have libSDL_mixer.])
  # temporaly put the CFLAGS for SDL in CPPFLAGS
  # so that AC_caolan_CHECK_PACKAGE can find SDL_mixer.h in the same directory.
  tmp_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$SDL_CFLAGS $CPPFLAGS"

  AC_caolan_CHECK_PACKAGE([SDL_mixer],[Mix_OpenAudio],
			  [SDL_mixer],[SDL_mixer.h],,
			  [with_sdl_mixer=no])

  # revert old value
  CPPFLAGS="$tmp_CPPFLAGS"
fi # with_sdl_mixer

if test "x${with_sdl_mixer}" != xno; then
  with_sdl_mixer=yes
  $1
ifelse([$2],,,[else
  $2])
fi # with_sdl_mixer
])
