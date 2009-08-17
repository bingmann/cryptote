# $Id$

bin_PROGRAMS = cryptote

cryptote_SOURCES = main.cpp \
	../common/tools.h \
	wcryptote.cpp wcryptote.h \
	bmpcat.cpp bmpcat.h \
	wtextpage.cpp wtextpage.h \
	wfind.cpp wfind.h \
	wfilelist.cpp wfilelist.h \
	wfileprop.cpp wfileprop.h \
	wcntprop.cpp wcntprop.h \
	wmsgdlg.cpp wmsgdlg.h \
	wbinpage.cpp wbinpage.h \
	wpass.cpp wpass.h \
	imaglbox.cpp imaglbox.h \
	wprefs.cpp wprefs.h \
	hhelpfs.cpp hhelpfs.h \
	../pwgen/wpassgen.cpp ../pwgen/wpassgen.h \
	../pwgen/fips181.cpp ../pwgen/fips181.h \
	../common/myintl.cpp ../common/myintl.h

cryptote_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a

dist_man_MANS = cryptote.1

AM_CXXFLAGS = -W -Wall "-DMY_BUILDTIME=\"`date -u '+%Y-%m-%d %H:%M %Z'`\"" -I$(top_srcdir)/src -I$(top_srcdir)/libenctain -I$(top_srcdir)/libstc $(WX_CXXFLAGS) $(BZLIB_CFLAGS) $(ZLIB_CFLAGS) 
LIBS = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a $(WX_LIBS) $(BZLIB_LIBS) $(ZLIB_LIBS)

if ON_WIN32
if GOT_RESCOMP

BUILT_SOURCES = cryptote-rc.o

cryptote-rc.o: cryptote.rc
	$(WX_RESCOMP) -I$(srcdir) $< $@

cryptote_LDFLAGS = cryptote-rc.o

endif
endif

if ON_MACOSX

# create an application bundle
cryptote.app/Contents/Pkginfo: cryptote$(EXEEXT) Info.plist
	mkdir -p cryptote.app/Contents/MacOS
	mkdir -p cryptote.app/Contents/Resources

	cp Info.plist cryptote.app/Contents/Info.plist
	/bin/echo -n "APPL????" > cryptote.app/Contents/PkgInfo
	cp $(top_srcdir)/src/art/cryptote-256.icns cryptote.app/Contents/Resources/
	cp $(top_srcdir)/src/art/ectfile-256.icns cryptote.app/Contents/Resources/
	cp cryptote$(EXEEXT) cryptote.app/Contents/MacOS/

all-local: cryptote.app/Contents/Pkginfo

clean-local:
	rm -rf cryptote.app

endif

EXTRA_DIST = cryptote.rc cryptote.xml \
	wabout.wxg wcntprop.wxg wfileprop.wxg \
	wfind.wxg wpass.wxg wprefs.wxg \
	Info.plist.in
