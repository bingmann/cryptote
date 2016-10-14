################################################################################
# Makefile.am
#
# Part of CryptoTE, see http://panthema.net/2007/cryptote
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

SUBDIRS = libenctain

if !HAVE_WXWIDGETS3
    SUBDIRS += libstc
endif

SUBDIRS += src desktop

EXTRA_DIST = \
	win32/cryptote-win32.iss.in \
	win32/cryptote-pwgen-win32.iss.in \
	win32/SetupModern20.bmp \
	win32/SetupModernSmall20.bmp \
	win32/COPYING.rtf

.PHONY: strip run-gcov

strip:
	strip -s src/cryptote/cryptote$(EXEEXT)
	strip -s src/pwgen/cryptote-pwgen$(EXEEXT)

if GCOV

clean-local:
	find -name "*.da" -o -name "*.gcov" -o -name "*.gcda" -o -name "*.gcno" | xargs rm || true

run-gcov: clean-gcov
	mkdir -p coverage/
	lcov --directory . --zerocounters
	-$(MAKE) check
	lcov --directory . --capture --output-file coverage/all.info
	lcov -e coverage/all.info "/tdata/*" -o coverage/testsuite.info
	genhtml -o coverage --num-spaces 8 coverage/testsuite.info --title "CryptoTE Testsuites" --prefix `pwd`

clean-gcov:
	find -name "*.gcda" | xargs rm || true

endif
