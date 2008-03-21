# $Id$

TESTS = test_ect1

noinst_PROGRAMS = test_ect1

test_ect1_SOURCES = test_ect1.cpp

test_ect1_DEPENDENCIES = ../libenctain.a

AM_CXXFLAGS = -W -Wall -I$(top_srcdir)/libenctain $(BZLIB_CFLAGS) $(ZLIB_CFLAGS) $(WX_CXXFLAGS)

LIBS = ../libenctain.a $(BZLIB_LIBS) $(ZLIB_LIBS) $(WX_LIBS)

if HAVE_LIBGCRYPT

TESTS += test_crc32 test_sha256 test_serpent_ecb test_serpent_cbc

noinst_PROGRAMS += test_crc32 test_sha256 test_serpent_ecb test_serpent_cbc

test_crc32_SOURCES = test_crc32.cpp

test_sha256_SOURCES = test_sha256.cpp

test_serpent_ecb_SOURCES = test_serpent_ecb.cpp

test_serpent_cbc_SOURCES = test_serpent_cbc.cpp

AM_CXXFLAGS += $(LIBGCRYPT_CFLAGS)
LIBS += $(LIBGCRYPT_LIBS)

endif

EXTRA_DIST = 