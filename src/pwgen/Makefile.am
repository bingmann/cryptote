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

if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o

pwgen-rc.o: pwgen.rc
	$(WX_RESCOMP) -I$(srcdir) $< $@

cryptote_pwgen_LDFLAGS = pwgen-rc.o

endif

EXTRA_DIST = pwgen.rc pwgen.xml \
	wpassgen.wxg
