#! /bin/sh
# Copyright 2000  Alexandre Duret-Lutz <duret_g@epita.fr>
#
# This file is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

if test -z $1; then
  echo "fetish-sync.sh FETISHDIR [DESTDIR]"
  exit 1
fi

if test -d $1/lib; then
  fetsrc="$1"
elif test -d $1/../lib; then
  fetsrc="$1/.."
else
  echo "can't find $1/lib"
  exit 2
fi

if test -z $2; then
  topsrc='.'
else
  topsrc="$2"
fi

if test -d $topsrc/lib; then
  :
elif test -d $topsrc/../lib; then
  $topsrc="$topsrc/.."
else
  echo "can't find $topsrc/lib"
  exit 3
fi

files='lib/error.c
lib/error.h
lib/getopt.c
lib/getopt1.c
lib/getopt.h:lib/gnugetopt.h
lib/hash.c
lib/hash.h
lib/malloc.c
lib/realloc.c
lib/strcasecmp.c
lib/strndup.c
lib/xalloc.h
lib/xmalloc.c
lib/xstrdup.c
m4/malloc.m4
m4/realloc.m4
m4/strerror_r.m4'

for f in $files; do
  case $f in
  *:*)
    dest=`echo $f | cut -d: -f2`
    src=`echo $f | cut -d: -f1` ;;
  *)
    dest=$f
    src=$f ;;
  esac
  echo "$fetsrc/$src -> $topsrc/$dest"
  cp $fetsrc/$src $topsrc/$dest
done
