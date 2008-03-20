# $Id$

if HAVE_LIBGCRYPT

noinst_PROGRAMS = speedtest_serpent

speedtest_serpent_SOURCES = speedtest_serpent.cpp

AM_CXXFLAGS = -W -Wall -O3 -I$(top_srcdir)/libenctain $(LIBGCRYPT_CFLAGS)

LIBS += ../libenctain.a $(LIBGCRYPT_LIBS)

endif

EXTRA_DIST = \
	speedtest.gnuplot \
	results-p3-1000/* \
	results-p4-3200/*
