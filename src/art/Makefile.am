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
	document_new.h \
	document_open.h \
	document_save.h \
	document_saveas.h \
	document_revert.h \
	document_close.h \
	application_exit.h \
	application_info.h \
	edit_undo.h \
	edit_redo.h \
	edit_cut.h \
	edit_copy.h \
	edit_paste.h \
	edit_clear.h \
	edit_find.h \
	edit_add.h \
	edit_remove.h \
	go_up.h \
	go_down.h \
	go_next.h \
	window_close.h \
	web-16.h

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
	document_new.png \
	document_open.png \
	document_save.png \
	document_saveas.png \
	document_revert.png \
	document_close.png \
	application_exit.png \
	application_info.png \
	edit_undo.png \
	edit_redo.png \
	edit_cut.png \
	edit_copy.png \
	edit_paste.png \
	edit_clear.png \
	edit_find.png \
	edit_add.png \
	edit_remove.png \
	go_up.png \
	go_down.png \
	go_next.png \
	window_close.png \
	web-16.png
