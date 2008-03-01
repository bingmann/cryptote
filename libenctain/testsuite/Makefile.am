# $Id$

TESTS =

noinst_PROGRAMS =

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libenctain $(WX_CXXFLAGS)

LIBS = ../libenctain.a $(WX_LIBS)

if HAVE_LIBGCRYPT

TESTS += test_crc32

noinst_PROGRAMS += test_crc32

test_crc32_SOURCES = test_crc32.cpp

AM_CXXFLAGS += $(LIBGCRYPT_CFLAGS)
LIBS += $(LIBGCRYPT_LIBS)

endif
