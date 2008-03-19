# $Id$

bin_PROGRAMS = pwgen

pwgen_SOURCES = main.cpp \
	wpassgen.cpp wpassgen.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h \
	../common/myintl.cpp ../common/myintl.h

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src $(WX_CXXFLAGS)
LIBS = $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o

pwgen-rc.o: pwgen.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwgen_LDFLAGS = pwgen-rc.o

endif
