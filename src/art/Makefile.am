# $Id$

noinst_PROGRAMS = file2h

file2h_SOURCES = file2h.cpp

AM_CFLAGS = -W -Wall
AM_CXXFLAGS = -W -Wall

%.h: %.png
	./file2h $< > $@

noinst_HEADERS = \
	pwgen-16.h \
	pwgen-22.h \
	pwgen-32.h \
	pwgen-48.h

EXTRA_DIST = \
	pwgen-16.png \
	pwgen-22.png \
	pwgen-32.png \
	pwgen-48.png \
	pwgen-128.png \
	pwgen.ico
