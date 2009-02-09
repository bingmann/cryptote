# $Id$

SUBDIRS = . html

cryptote.pdf: cryptote.tex
	pdflatex -halt-on-error $<
	pdflatex -halt-on-error $<

pdf: cryptote.pdf

html/cryptote.hhp: cryptote.tex
	tex2rtf cryptote.tex html/cryptote -html

html: html/cryptote.hhp

.PHONY: pdf html

all-local: html/cryptote.hhp

EXTRA_DIST =  cryptote.tex tex2rtf.ini texhelp.sty

CLEANFILES = cryptote.aux cryptote.idx cryptote.log cryptote.toc cryptote.out cryptote.pdf

clean-local:
	rm -f *.con
