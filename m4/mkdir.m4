dnl AC_FUNC_MKDIR
dnl Check for mkdir.
dnl Can define HAVE_MKDIR, HAVE__MKDIR and MKDIR_TAKES_ONE_ARG.
dnl
dnl #if HAVE_MKDIR
dnl # if MKDIR_TAKES_ONE_ARG
dnl    /* Mingw32 */
dnl #  define mkdir(a,b) mkdir(a)
dnl # endif
dnl #else
dnl # if HAVE__MKDIR
dnl    /* plain Win32 */
dnl #  define mkdir(a,b) _mkdir(a)
dnl # else
dnl #  error "Don't know how to create a directory on this system."
dnl # endif
dnl #endif
dnl
dnl Written by Alexandre Duret-Lutz <duret_g@epita.fr>.

AC_DEFUN([AC_FUNC_MKDIR],
[AC_CHECK_FUNCS([mkdir _mkdir])
AC_CACHE_CHECK([whether mkdir takes one argument],
                [ac_cv_mkdir_takes_one_arg],
[AC_TRY_COMPILE([
#include <sys/stat.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
],[mkdir (".");],
[ac_cv_mkdir_takes_one_arg=yes],[ac_cv_mkdir_takes_one_arg=no])])
if test x"$ac_cv_mkdir_takes_one_arg" = xyes; then
  AC_DEFINE([MKDIR_TAKES_ONE_ARG],1,
            [Define if mkdir takes only one argument.])
fi
])
