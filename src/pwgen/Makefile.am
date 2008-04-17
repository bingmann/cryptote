# $Id$

noinst_PROGRAMS = cryptote-pwgen

cryptote_pwgen_SOURCES = main.cpp \
	../common/tools.h \
	wpassgen.cpp wpassgen.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h \
	../common/myintl.cpp ../common/myintl.h

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src $(WX_CXXFLAGS) $(ZLIB_CFLAGS)
LIBS = $(WX_LIBS) $(ZLIB_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o

pwgen-rc.o: pwgen.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_pwgen_LDFLAGS = pwgen-rc.o

endif

EXTRA_DIST = pwgen.rc pwgen.xml \
	wpassgen.wxg
