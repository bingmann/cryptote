################################################################################
# src/art/Makefile.am
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

SUBDIRS = . crystal slick gnome

noinst_PROGRAMS = file2h

file2h_SOURCES = file2h.cpp

AM_CFLAGS = -W -Wall $(ZLIB_CFLAGS)
AM_CXXFLAGS = -W -Wall $(ZLIB_CFLAGS)
LIBS = $(ZLIB_LIBS) $(EXTRA_LIBS)

if !CROSS_COMPILATION
%.h: %.png
	./file2h $< > $@
endif

noinst_HEADERS = \
	pwgen-16.h \
	pwgen-22.h \
	pwgen-32.h \
	pwgen-48.h \
	cryptote-16.h \
	cryptote-32.h \
	cryptote-48.h \
	ectfile-16.h \
	ectfile-32.h \
	ectfile-48.h \
	modified-12.h \
	modified-16.h \
	unmodified-16.h \
	web-16.h

EXTRA_DIST = \
	pwgen-16.png \
	pwgen-22.png \
	pwgen-32.png \
	pwgen-48.png \
	pwgen-128.png \
	pwgen-128.icns \
	pwgen.ico \
	cryptote-16.png \
	cryptote-32.png \
	cryptote-48.png \
	cryptote-256.icns \
	cryptote.ico \
	cryptote.svg \
	ectfile-16.png \
	ectfile-32.png \
	ectfile-48.png \
	ectfile-256.icns \
	ectfile.ico \
	ectfile.svg \
	modified-12.png \
	modified-12.svg \
	modified-16.png \
	unmodified-16.png \
	web-16.png
