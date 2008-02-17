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
	pwgen-48.h \
	pwtutor-16.h \
	pwtutor-22.h \
	pwtutor-32.h \
	pwtutor-48.h \
	edit_add.h \
	edit_remove.h

EXTRA_DIST = \
	pwgen-16.png \
	pwgen-22.png \
	pwgen-32.png \
	pwgen-48.png \
	pwgen-128.png \
	pwgen.ico \
	pwtutor-16.png \
	pwtutor-22.png \
	pwtutor-32.png \
	pwtutor-48.png \
	pwtutor-128.png \
	pwtutor.ico \
	edit_add.png \
	edit_remove.png
