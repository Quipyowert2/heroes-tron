#! /bin/sh
# Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>
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

BEGIN {
  output = 0;
  print "/*";
  print "** Copyright 2002  Alexandre Duret-Lutz <duret_g@epita.fr>";
  print "**";
  print "** This file is free software; you can redistribute it and/or modify";
  print "** it under the terms of the GNU General Public License as published";
  print "** by the Free Software Foundation; either version 2 of the License,";
  print "** or (at your option) any later version.";
  print "**";
  print "** This program is distributed in the hope that it will be useful,";
  print "** but WITHOUT ANY WARRANTY; without even the implied warranty of";
  print "** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU";
  print "** General Public License for more details.";
  print "**";
  print "** You should have received a copy of the GNU General Public License";
  print "** along with this program; if not, write to the Free Software";
  print "** Foundation, Inc., 59 Temple Place, Suite 330, Boston,";
  print "** MA 02111-1307 USA";
  print "*/";
  print;
  print "/*-----------------------------------------------------------.";
  print "| DO NOT MODIFY.  This file contains portions extracted from |";
  print "| several source files.  Change these files instead.         |";
  print "`-----------------------------------------------------------*/";
  print;
}

/END PUBLIC/ {
  output = 0;
}

{
  if (output)
    print $0;
}

/BEGIN PUBLIC/ {
  output = 1;
  print "/*****************************************************************";
  print "******************************************************************";
  print "*** extracted from", FILENAME;
  print "**/";
}
