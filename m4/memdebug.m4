dnl Some macros to configure with malloc() debugging libraries.
dnl
dnl Main functions:
dnl  adl_WITH_DMALLOC		handle the --with-dmalloc option
dnl  adl_WITH_EFENCE		handle the --with-efence option
dnl  adl_ENABLE_MEM_DEBUG	handle the --enable-mem-debug option
dnl				(i.e. link with either dmalloc or efence)
dnl
dnl Auxiliary functions:
dnl  adl_CHECK_DMALLOC		actually check for dmalloc availability
dnl  adl_CHECK_EFENCE		actually check for efence avalability
dnl
dnl Written by Alexandre Duret-Lutz <duret_g@epita.fr>


dnl adl_CHECK_DMALLOC ([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Check for dmalloc.
dnl
dnl This macro defines HAVE_DMALLOC_H when the dmalloc.h header is found.
dnl dmalloc.h is not mandatory to use dmalloc, but it is better to
dnl include it when available.  Here is how:
dnl
dnl /* keep this header at the end of the include list, because it may
dnl    define some macros to change the declaration of malloc functions */
dnl #if HAVE_DMALLOC_H
dnl # define DMALLOC_FUNC_CHECK
dnl # include <dmalloc.h>
dnl #endif

AC_DEFUN([adl_CHECK_DMALLOC],
 [test_failed='no'
  # should we check for dmalloc?
  if test "${with_dmalloc}" = no; then
    test_failed='yes'
  else
  # see if dmalloc is usable
    AC_CHECK_LIB([dmalloc],[malloc],
     [ifelse([$1],,,[$1])
      LIBS="-ldmalloc $LIBS"
      AC_CHECK_HEADER([dmalloc.h])],
     [ifelse([$2],,[AC_MSG_ERROR([Cannot find dmalloc])],[test_failed='yes'])])
  fi
  ifelse([$2],,,
   [if test "$test_failed" = yes; then
     $2;
    fi])])


dnl adl_WITH_DMALLOC ([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Check for dmalloc on user request.

AC_DEFUN([adl_WITH_DMALLOC],
 [AC_ARG_WITH([dmalloc],
  [AC_HELP_STRING([--with-dmalloc],
   [link with dmalloc (a malloc() debugger)])])
  if test "$with_dmalloc" = yes; then
   adl_CHECK_DMALLOC
  fi])


dnl adl_CHECK_EFENCE ([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Check for efence.

AC_DEFUN([adl_CHECK_EFENCE],
 [test_failed='no'
  # should we check for efence?
  if test "${with_efence}" = no; then
    test_failed='yes'
  else
  # see if efence is usable
    AC_CHECK_LIB([efence],[malloc],
     [ifelse([$1],,,[$1])
      LIBS="-lefence $LIBS"],
     [ifelse([$2],,[AC_MSG_ERROR([Cannot find Electric Fence])],
      [test_failed='yes'])])
  fi
  ifelse([$2],,,
   [if test "$test_failed" = yes; then
     $2;
    fi])])


dnl adl_WITH_EFENCE ([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Check for efence on user request.

AC_DEFUN([adl_WITH_EFENCE],
 [AC_ARG_WITH([efence],
  [AC_HELP_STRING([--with-efence],
   [link with efence (a malloc() debugger)])])
  if test "$with_efence" = yes; then
   adl_CHECK_EFENCE
  fi])


dnl adl_ENABLE_MEM_DEBUG
dnl Check for dmalloc or efence on request, use the first library found.

AC_DEFUN([adl_ENABLE_MEM_DEBUG],
 [# don't test anything if --with-dmalloc or --with-efence
  # was given (assume that adl_WITH_DMALLOC or adl_WITH_EFENCE
  # has already run).
  if test x"${with_dmalloc-no}" = xno -a x"${with_efence-no}" = xno; then
    AC_ARG_ENABLE([mem-debug],
    [AC_HELP_STRING([--enable-mem-debug],
     [link with any malloc() debugger available])])
    if test "$enable_mem_debug" = yes; then
     adl_CHECK_DMALLOC(,[adl_CHECK_EFENCE(,[mallocdbg="\<none\>"])])
    fi
  fi])
