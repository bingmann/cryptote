# $Id$

bin_PROGRAMS = cryptote

cryptote_SOURCES = main.cpp \
	wcryptote.cpp wcryptote.h \
	wtextpage.cpp wtextpage.h

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src -I$(top_srcdir)/libstc $(WX_CXXFLAGS)
LIBS = $(top_builddir)/libstc/libstc.a $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = cryptote-rc.o

cryptote-rc.o: cryptote.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_LDFLAGS = cryptote-rc.o

endif
