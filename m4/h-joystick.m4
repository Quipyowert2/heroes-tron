AC_DEFUN([heroes_CHECK_JOYSTICK_LIB],
[AC_ARG_ENABLE([joystick],
  [AC_HELP_STRING([--disable-joystick],[turn off joystick support])])

joystick_lib='<disabled>'
if test "${enable_joystick-yes}" = yes; then

  # -- check for Joystick support in SDL

  sdl_joystick=no
  if test "${with_sdl-yes}" != no && test "${with_gii-no}" = no;  then
    AC_CHECK_FUNCS([SDL_JoystickOpen],[
sdl_joystick=yes
joystick_lib="SDL dnl
$sdl_config_major_version.$sdl_config_minor_version.$sdl_config_micro_version"
])
  fi

  # -- check for LibGII (another way to get joystick support)

  gii_joystick=no
  if test $sdl_joystick = no && test "${with_gii-yes}" != no;  then
    AC_caolan_CHECK_PACKAGE([GII],[giiOpen],[gii],[ggi/gii.h],[
gii_joystick=yes
joystick_lib='LibGII'
])
  fi

  if test $sdl_joystick = no && test $gii_joystick = no; then
    enable_joystick=no
  fi
fi

if test "${enable_joystick-yes}" = yes; then
  AC_DEFINE([JOYSTICK_SUPPORT],1,[Define if you want joystick support.])
fi
])
