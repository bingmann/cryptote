# $Id$

bin_PROGRAMS = cryptote

cryptote_SOURCES = main.cpp \
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
	wprefs.cpp wprefs.h \
	../pwgen/wpassgen.cpp ../pwgen/wpassgen.h \
	../pwgen/prng.cpp ../pwgen/prng.h \
	../pwgen/fips181.cpp ../pwgen/fips181.h \
	../common/myintl.cpp ../common/myintl.h

cryptote_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src -I$(top_srcdir)/libenctain -I$(top_srcdir)/libstc $(BZLIB_CFLAGS) $(ZLIB_CFLAGS) $(WX_CXXFLAGS)
LIBS = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a $(BZLIB_LIBS) $(ZLIB_LIBS) $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = cryptote-rc.o

cryptote-rc.o: cryptote.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_LDFLAGS = cryptote-rc.o

endif
