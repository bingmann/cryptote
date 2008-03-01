# $Id$

if HAVE_LIBGCRYPT

noinst_PROGRAMS = speedtest_serpent

speedtest_serpent_SOURCES = speedtest_serpent.cpp

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libenctain $(LIBGCRYPT_CFLAGS)

LIBS += ../libenctain.a $(LIBGCRYPT_LIBS)

endif
