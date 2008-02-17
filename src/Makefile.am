# $Id$

SUBDIRS = art .

bin_PROGRAMS = cryptote pwtutor pwgen

cryptote_SOURCES = main.cpp \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h

pwtutor_SOURCES = pwtutor.cpp \
	ptwmain.cpp ptmwain.h \
	ptwnew.cpp ptwnew.h \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h

pwgen_SOURCES = pwgen.cpp \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h

AM_CFLAGS = -W -Wall $(WX_CFLAGS)
AM_CXXFLAGS = -W -Wall $(WX_CXXFLAGS)
LIBS = $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o pwtutor-rc.o

pwgen-rc.o: pwgen.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwtutor-rc.o: pwtutor.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwgen_LDFLAGS = pwgen-rc.o

pwtutor_LDFLAGS = pwtutor-rc.o

endif
