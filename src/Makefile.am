# $Id$

SUBDIRS = art .

bin_PROGRAMS = cryptote pwgen

cryptote_SOURCES = main.cpp \
	wgenpass.cpp wgenpass.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h

pwgen_SOURCES = pwgen.cpp \
	wgenpass.cpp wgenpass.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h

AM_CFLAGS = -W -Wall $(WX_CFLAGS)
AM_CXXFLAGS = -W -Wall $(WX_CXXFLAGS)
LIBS = $(WX_LIBS)

if GOT_RESCOMP
BUILT_SOURCES = pwgen-rc.o

pwgen-rc.o: pwgen.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwgen_LDFLAGS = pwgen-rc.o
endif
