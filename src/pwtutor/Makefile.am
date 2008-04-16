# $Id$

noinst_PROGRAMS = cryptote-pwtutor

cryptote_pwtutor_SOURCES = main.cpp \
	../common/tools.h \
	wpwtutor.cpp wpwtutor.h \
	wnewpass.cpp wnewpass.h \
	wquery.cpp wquery.h \
	wstats.cpp wstats.h \
	../pwgen/wpassgen.cpp ../pwgen/wpassgen.h \
	../pwgen/prng.cpp ../pwgen/prng.h \
	../pwgen/fips181.cpp ../pwgen/fips181.h

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src $(WX_CXXFLAGS)
LIBS = $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = pwtutor-rc.o

pwtutor-rc.o: pwtutor.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_pwtutor_LDFLAGS = pwtutor-rc.o

endif

EXTRA_DIST = pwtutor.rc pwtutor.xml \
	wnewpass.wxg wpwtutor.wxg wquery.wxg \
	wstats.wxg
