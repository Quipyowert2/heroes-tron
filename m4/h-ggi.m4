AC_DEFUN([heroes_CHECK_GGI],
[if test "${with_ggi-yes}" != no; then
   # Provide a config.h help string, as AC_caolan_CHECK_PACKAGE does not.
   AH_TEMPLATE([HAVE_PKG_GGI],[Define if you have libggi.])

   AC_caolan_CHECK_PACKAGE([GGI],[ggiOpen],[ggi],[ggi/ggi.h],,[with_ggi=no])
 fi
 if test "${with_ggi-yes}" != no; then
   AC_adl_FIND_HEADER([ggi/keyboard.h],,
     [/usr/include/ggi/keyboard.h] dnl
     [${ggi_include-/usr/local/include}/ggi/keyboard.h],
     [#[ 	]*define[ 	][ 	]*GIIUC*],7,,
     [AC_MSG_ERROR([Keysyms macros not found: where is GGI installed?])])

   GGI_KEYBOARD_H="$ac_cv_header_path_ggi_keyboard_h"
   AC_SUBST([GGI_KEYBOARD_H])
   AM_CONDITIONAL(GGI, [test "x${with_ggi}" != xno])
   $1
 ifelse([$2],,,[else
   $2])
 fi])
