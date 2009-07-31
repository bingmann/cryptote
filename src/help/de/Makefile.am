# $Id$

SUBDIRS = . html

cryptote.pdf: cryptote.tex
	pdflatex -halt-on-error $<
	pdflatex -halt-on-error $<

pdf: cryptote.pdf

if GOT_TEX2RTF

html/cryptote.hhp: cryptote.tex
	LANG=de_DE tex2rtf cryptote.tex html/cryptote -html

html: html/cryptote.hhp

all-local: html/cryptote.hhp

endif

.PHONY: pdf html

EXTRA_DIST =  cryptote.tex tex2rtf.ini texhelp.sty

CLEANFILES = cryptote.aux cryptote.idx cryptote.log cryptote.toc cryptote.out cryptote.pdf

clean-local:
	rm -f *.con
