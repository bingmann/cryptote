# $Id$

SUBDIRS = art .

bin_PROGRAMS = cryptote pwtutor pwgen

cryptote_SOURCES = cryptote.cpp \
	cewmain.cpp cewmain.h \
	cewedit.cpp cewedit.h \
	cewfind.cpp cewfind.h \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h \
	hyperlink.cpp hyperlink.h

pwtutor_SOURCES = pwtutor.cpp \
	ptwmain.cpp ptmwain.h \
	ptwnew.cpp ptwnew.h \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h \
	hyperlink.cpp hyperlink.h \
	ptwquery.cpp ptwquery.h \
	ptwstats.cpp ptwstats.h

pwgen_SOURCES = pwgen.cpp \
	pgwmain.cpp pgwmain.h \
	prng.cpp prng.h \
	fips181.cpp fips181.h \
	hyperlink.cpp hyperlink.h

AM_CFLAGS = -W -Wall -I$(top_srcdir)/libstc $(WX_CFLAGS)
AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libstc $(WX_CXXFLAGS)
LIBS = $(top_builddir)/libstc/libstc.a $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = pwgen-rc.o pwtutor-rc.o

pwgen-rc.o: pwgen.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwtutor-rc.o: pwtutor.rc
	$(RESCOMP) -I$(srcdir) $< $@

pwgen_LDFLAGS = pwgen-rc.o

pwtutor_LDFLAGS = pwtutor-rc.o

endif
