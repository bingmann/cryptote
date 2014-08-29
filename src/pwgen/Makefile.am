################################################################################
# src/pwgen/Makefile.am
#
# Part of CryptoTE v0.5.999, see http://panthema.net/2007/cryptote
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

noinst_PROGRAMS = cryptote-pwgen

cryptote_pwgen_SOURCES = main.cpp \
	../common/tools.h \
	wpassgen.cpp wpassgen.h \
	fips181.cpp fips181.h \
	../common/myintl.cpp ../common/myintl.h

cryptote_pwgen_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a 

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src -I$(top_srcdir)/libenctain $(WX_CXXFLAGS) $(BZLIB_CFLAGS) $(ZLIB_CFLAGS)
LIBS = $(top_builddir)/libenctain/libenctain.a  $(WX_LIBS) $(BZLIB_LIBS) $(ZLIB_LIBS) $(EXTRA_LIBS)

if ON_WIN32
if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o

pwgen-rc.o: pwgen.rc
	$(WX_RESCOMP) -I$(srcdir) $< $@

cryptote_pwgen_LDFLAGS = pwgen-rc.o

endif
endif

if ON_MACOSX

# create an application bundle
cryptote-pwgen.app/Contents/Pkginfo: cryptote-pwgen$(EXEEXT) Info.plist
	mkdir -p cryptote-pwgen.app/Contents/MacOS
	mkdir -p cryptote-pwgen.app/Contents/Resources

	cp Info.plist cryptote-pwgen.app/Contents/Info.plist
	/bin/echo -n "APPL????" >cryptote-pwgen.app/Contents/PkgInfo
	cp $(top_srcdir)/src/art/pwgen-128.icns cryptote-pwgen.app/Contents/Resources/
	cp cryptote-pwgen$(EXEEXT) cryptote-pwgen.app/Contents/MacOS/

all-local: cryptote-pwgen.app/Contents/Pkginfo

clean-local:
	rm -rf cryptote-pwgen.app

endif

EXTRA_DIST = pwgen.rc pwgen.xml \
	wpassgen.wxg Info.plist.in
