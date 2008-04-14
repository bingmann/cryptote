# $Id$

SUBDIRS = . testsuite speedtest

noinst_LIBRARIES = libenctain.a

libenctain_a_SOURCES = bytebuff.h \
	crc32.cpp crc32.h \
	sha256.cpp sha256.h \
	serpent.cpp serpent.h \
	enctain.cpp enctain.h

AM_CXXFLAGS = -W -Wall $(ZLIB_CFLAGS) $(BZLIB_CFLAGS)

EXTRA_DIST = format.html