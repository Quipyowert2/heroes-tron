AC_DEFUN([adl_ENABLE_DEBUG],
 [AC_ARG_ENABLE([debug],
  [AC_HELP_STRING([--enable-debug],[enable debugging symbols])])
  if test "x${enable_debug}" = xyes; then
   AC_DEFINE([DEBUG],1,[Define if you want debugging code.])
   if test -n "$GCC"; then
    CFLAGS="$CFLAGS -ggdb3"
   else
    CFLAGS="$CFLAGS -g"
   fi
  fi])
