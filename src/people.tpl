[~ autogen5 template
  authors=AUTHORS
  thanks=THANKS
  texi=people.texi
# Copyright 2001  Alexandre Duret-Lutz <duret_g@epita.fr>
#
# This file is part of Heroes.
#
# Heroes is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# Heroes is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
~]
[~ (define (email) (sprintf "<%s>" (get "email")))
   (define (char-to-texi c)
      (let
       ((res (case c
	       ((#\å) "@aa{}") ((#\à) "@`a")  ((#\ä) "@\"a")  ((#\â) "@^a")
	       ((#\é) "@'e")   ((#\è) "@`e")  ((#\ë) "@\"e")  ((#\ê) "@^e")
	       ((#\í) "@'i")   ((#\ì) "@`i")  ((#\ï) "@\"i")  ((#\î) "@^i")
	       ((#\ç) "@,{c}") ((#\ñ) "@~n")  ((#\ß) "@ss{}") ((#\@) "@@")
	       (else c))))
	(if (string? res) (string->list res) (list res))))
   (define (to-texi string-to-recode)
      (list->string
        (apply append (map char-to-texi (string->list string-to-recode)))))
   (define (name-texi) (to-texi (get "name")))
   (define (email-texi) (to-texi (get "email")))
   (define (i18n-texi) (to-texi (get "i18n")))
   (define (package-texi) (to-texi (get "package")))
   (define (author-texi) (to-texi (get "author")))
   (define (port-texi) (to-texi (get "port")))
   (define (contrib-texi) (to-texi (get "contrib")))
~][~
  CASE (suffix) ~][~
    == authors ~][~
      FOR person ~][~
        IF author
          ~]* [~ name ~] [~ IF aka ~]([~ aka ~]) [~ ENDIF ~][~ (email) ~]
[~        IF www ~]  [~ www ~]
[~        ENDIF www ~]
[~        (out-push-new ".author") ~][~ author ~][~ (out-pop ) ~][~
          `sed -e 's/^/	/' .author; rm -f .author` ~]

[~      ENDIF author ~][~
      ENDFOR person ~][~
    == thanks ~]This file records people who contributed to Heroes by reporting
problems, suggesting various improvements or submitting actual code.
More details about the contributions can be found in the manual
or the ChangeLog.  If your name has been left out, if you'd rather
not be listed, or if you'd prefer a different address be used, please
send a note to <heroes-bugs@lists.sourceforge.net>.

[~    FOR person ~][~
        IF author ~][~
        ELSE ~][~ (sprintf "%-38s  %s\n" (get "name") (get "email")) ~][~
        ENDIF ~][~
      ENDFOR person ~][~
    == texi ~]
@unnumberedsubsec Authors
@itemize @bullet
[~    FOR person ~][~
        IF author ~]@item
[~        IF email ~]@email{[~ (email-texi) ~],[~ (name-texi) ~]}[~
          ELSE ~][~ (name-texi) ~][~
          ENDIF ~]

[~ (author-texi) ~]

[~      ENDIF ~][~
      ENDFOR person ~]@end itemize

@unnumberedsubsec Contributors
@itemize @bullet
[~    FOR person ~][~
        IF contrib ~]@item
[~        IF email ~]@email{[~ (email-texi) ~],[~ (name-texi) ~]}[~
          ELSE ~][~ (name-texi) ~][~
          ENDIF ~]

[~ (contrib-texi) ~]

[~      ENDIF ~][~
      ENDFOR person ~]@end itemize

@unnumberedsubsec Translators
@itemize @bullet
[~    FOR person ~][~
        IF i18n ~]@item
[~        IF email ~]@email{[~ (email-texi) ~],[~ (name-texi) ~]}[~
          ELSE ~][~ (name-texi) ~][~
          ENDIF ~] ([~ (i18n-texi) ~])
[~      ENDIF ~][~
      ENDFOR person ~]@end itemize

@unnumberedsubsec Packagers
@itemize @bullet
[~    FOR person ~][~
        IF package ~]@item
[~        IF email ~]@email{[~ (email-texi) ~],[~ (name-texi) ~]}[~
          ELSE ~][~ (name-texi) ~][~
          ENDIF ~] ([~ (package-texi) ~])
[~      ENDIF ~][~
      ENDFOR person ~]@end itemize

@unnumberedsubsec Porters
@itemize @bullet
[~    FOR person ~][~
        IF port ~]@item
[~        IF email ~]@email{[~ (email-texi) ~],[~ (to-texi (get "name")) ~]}[~
          ELSE ~][~ (name-texi) ~][~
          ENDIF ~] ([~ (port-texi) ~])
[~      ENDIF ~][~
      ENDFOR person ~]@end itemize

[~ ESAC ~]