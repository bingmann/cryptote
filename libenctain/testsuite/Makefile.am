# $Id$

TESTS = test_ect1 test_ect2 test_sha256 test_serpent

noinst_PROGRAMS = test_ect1 test_ect2 test_sha256 test_serpent

test_ect1_SOURCES = test_ect1.cpp

test_ect1_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_ect2_SOURCES = test_ect2.cpp

test_ect2_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_sha256_SOURCES = test_sha256.cpp

test_sha256_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

test_serpent_SOURCES = test_serpent.cpp

test_serpent_DEPENDENCIES = $(top_builddir)/libenctain/libenctain.a

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libenctain $(BZLIB_CFLAGS) $(ZLIB_CFLAGS)

LIBS = $(top_builddir)/libenctain/libenctain.a $(BZLIB_LIBS) $(ZLIB_LIBS)

EXTRA_DIST = frozen1.ect
