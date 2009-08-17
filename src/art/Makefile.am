# $Id$

SUBDIRS = . crystal slick gnome

noinst_PROGRAMS = file2h

file2h_SOURCES = file2h.cpp

AM_CFLAGS = -W -Wall $(ZLIB_CFLAGS)
AM_CXXFLAGS = -W -Wall $(ZLIB_CFLAGS)
LIBS = $(ZLIB_LIBS)

%.h: %.png
	./file2h $< > $@

noinst_HEADERS = \
	pwgen-16.h \
	pwgen-22.h \
	pwgen-32.h \
	pwgen-48.h \
	cryptote-16.h \
	cryptote-32.h \
	cryptote-48.h \
	ectfile-16.h \
	ectfile-32.h \
	ectfile-48.h \
	modified-12.h \
	modified-16.h \
	unmodified-16.h \
	web-16.h

EXTRA_DIST = \
	pwgen-16.png \
	pwgen-22.png \
	pwgen-32.png \
	pwgen-48.png \
	pwgen-128.png \
	pwgen-128.icns \
	pwgen.ico \
	cryptote-16.png \
	cryptote-32.png \
	cryptote-48.png \
	cryptote-256.icns \
	cryptote.ico \
	cryptote.svg \
	ectfile-16.png \
	ectfile-32.png \
	ectfile-48.png \
	ectfile-256.icns \
	ectfile.ico \
	ectfile.svg \
	modified-12.png \
	modified-12.svg \
	modified-16.png \
	unmodified-16.png \
	web-16.png
