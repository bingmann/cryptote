# $Id$

SUBDIRS = art pwgen pwtutor .

bin_PROGRAMS = cryptote

cryptote_SOURCES = cryptote.cpp \
	cewmain.cpp cewmain.h \
	ceweditctrl.cpp ceweditctrl.h \
	cewfind.cpp cewfind.h \
	pwgen/wpassgen.cpp pwgen/wpassgen.h \
	pwgen/prng.cpp pwgen/prng.h \
	pwgen/fips181.cpp pwgen/fips181.h \
	common/hyperlink.cpp common/hyperlink.h \
	wmsgdlg.cpp wmsgdlg.h

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libstc $(WX_CXXFLAGS)
LIBS = $(top_builddir)/libstc/libstc.a $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = cryptote-rc.o

cryptote-rc.o: cryptote.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_LDFLAGS = cryptote-rc.o

endif
