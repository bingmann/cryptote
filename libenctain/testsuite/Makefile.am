################################################################################
# libenctain/testsuite/Makefile.am
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

TESTS = test_ect1 test_ect2 test_sha256 test_serpent

noinst_PROGRAMS = test_ect1 test_ect2 test_sha256 test_serpent

test_ect1_SOURCES = test_ect1.cpp

test_ect1_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_ect2_SOURCES = test_ect2.cpp

test_ect2_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_sha256_SOURCES = test_sha256.cpp

test_sha256_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_serpent_SOURCES = test_serpent.cpp

test_serpent_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libenctain $(BZLIB_CFLAGS) $(ZLIB_CFLAGS)

LIBS = $(top_builddir)/libenctain/libenctain.a $(BZLIB_LIBS) $(ZLIB_LIBS) $(EXTRA_LIBS)

EXTRA_DIST = frozen1.ect
