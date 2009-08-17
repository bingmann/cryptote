# $Id$

noinst_PROGRAMS = cryptote-pwgen

cryptote_pwgen_SOURCES = main.cpp \
	../common/tools.h \
	wpassgen.cpp wpassgen.h \
	fips181.cpp fips181.h \
	../common/myintl.cpp ../common/myintl.h

cryptote_pwgen_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a 

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src -I$(top_srcdir)/libenctain $(WX_CXXFLAGS) $(BZLIB_CFLAGS) $(ZLIB_CFLAGS)
LIBS = $(top_builddir)/libenctain/libenctain.a  $(WX_LIBS) $(BZLIB_LIBS) $(ZLIB_LIBS)

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
