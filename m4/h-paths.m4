AC_DEFUN([heroes_COMPUTE_PATHS],
[AC_REQUIRE([AM_WITH_NLS])dnl

# DATADIRNAME is set to share (or lib) by AM_GNU_GETTEXT, this has nothing
# to do with datadir.  If heroes is configured with --prefix=/usr
# --bindir=/usr/games --datadir=/usr/share/games as the HFS requests,
# locale data will go into /usr/share/locale.  This comply with HFS.
localedir='${prefix}'"/$DATADIRNAME/locale"

pkgdatadir="$datadir/heroes"
adl_COMPUTE_RELATIVE_PATHS([dnl
bindir:prefix:backward_relative_bindir dnl
prefix:pkgdatadir:forward_relative_pkgdatadir dnl
prefix:localedir:forward_relative_localedir dnl
])

AC_DEFINE_UNQUOTED([BACKWARD_RELATIVE_BINDIR],
                   ["$backward_relative_bindir"],
                   [Relative path from BINDIR to PREFIX.])
AC_DEFINE_UNQUOTED([FORWARD_RELATIVE_PKGDATADIR],
                   ["$forward_relative_pkgdatadir"],
                   [Relative path from PREFIX to PKGDATADIR.])
AC_DEFINE_UNQUOTED([FORWARD_RELATIVE_LOCALEDIR],
                   ["$forward_relative_localedir"],
                   [Relative path from PREFIX to LOCALEDIR.])
AC_DEFINE_DIR([PREFIX],[prefix],[Installation prefix path.])
])
