AC_DEFUN([heroes_CHECK_MIKMOD_PRELIM],
[# --with-mikmod-exec-prefix should imply --with-mikmod, and vice-versa

test "${with_mikmod_exec_prefix-no}" != no &&
 test "${with_mikmod-yes}" = yes && with_mikmod="$with_mikmod_exec_prefix"

test "${with_mikmod-yes}" != yes &&
 test "${with_mikmod_exec_prefix-yes}" = yes &&
   with_mikmod_exec_prefix="$with_mikmod"
])

AC_DEFUN([heroes_CHECK_MIKMOD],
[AC_ARG_WITH([mikmod],
 [AC_HELP_STRING([--with-mikmod=DIR],
                 [root directory of LibMikMod installation])
AC_HELP_STRING([--without-mikmod],
               [disables LibMikMod usage completely])])

 if test "${with_mikmod-yes}" != no; then
   if test "${with_mikmod-no}" != no; then
     user_selection_list_s="$user_selection_list_s miks"
   fi
   AC_adl_PKG_GENERIC([LibMikMod],[3.1.7],[MikMod_Init],
   [AC_DEFINE([HAVE_LIBMIKMOD],1,[Define if you have the LibMikMod library.])],
   [with_mikmod=no])
 fi
 if test "${with_mikmod-yes}" != no; then
   ifelse([$1],,[:],[$1])
 ifelse([$2],,,[else
   $2])
 fi])
