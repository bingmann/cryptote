# $Id$

bin_PROGRAMS = cryptote

cryptote_SOURCES = main.cpp \
	wcryptote.cpp wcryptote.h \
	wtextpage.cpp wtextpage.h \
	wfind.cpp wfind.h \
	wfilelist.cpp wfilelist.h

cryptote_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/src -I$(top_srcdir)/libenctain -I$(top_srcdir)/libstc $(WX_CXXFLAGS)
LIBS = $(top_builddir)/libenctain/libenctain.a $(top_builddir)/libstc/libstc.a $(WX_LIBS)

if GOT_RESCOMP

BUILT_SOURCES = cryptote-rc.o

cryptote-rc.o: cryptote.rc
	$(RESCOMP) -I$(srcdir) $< $@

cryptote_LDFLAGS = cryptote-rc.o

endif
