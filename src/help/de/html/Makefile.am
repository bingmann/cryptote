################################################################################
# src/help/de/html/Makefile.am
#
# Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote
#
# Copyright (C) 2008-2014 Timo Bingmann <tb@panthema.net>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

if !CROSS_COMPILATION
%.h: %
	../../../art/file2h -c -p "help-de-" $< > $@

cryptote.hhp.cached: cryptote.hhp
	hhp2cached $<
endif

cleanmore:
	rm -f *.html *.hhc *.hhk *.hhp *.hhp.cached *.htx *.con *.ref *.h

noinst_HEADERS = \
	back.gif.h \
	contents.gif.h \
	forward.gif.h \
	up.gif.h \
	cryptote.hhc.h \
	cryptote.hhk.h \
	cryptote.hhp.h \
	cryptote.hhp.cached.h \
	cryptote_contents.html.h \
	cryptote_einfuehrung.html.h \
	cryptote_ueberverschluesselung.html.h \
	cryptote_funktionsumfang.html.h

EXTRA_DIST = \
	style.css \
	back.gif \
	contents.gif \
	forward.gif \
	up.gif \
	cryptote.hhc \
	cryptote.hhk \
	cryptote.hhp \
	cryptote.hhp.cached \
	cryptote_contents.html \
	cryptote_einfuehrung.html \
	cryptote_ueberverschluesselung.html \
	cryptote_funktionsumfang.html
