# $Id$

%.h: %
	../../art/file2h -c -p "help-en-" $< > $@

cryptote.hhp.cached: cryptote.hhp
	hhp2cached $<

cleanmore:
	rm -f *.html *.hhc *.hhk *.hhp *.hhp.cached *.htx *.con *.ref *.h

noinst_HEADERS = \
	back.gif.h \
	contents.gif.h \
	forward.gif.h \
	up.gif.h \
	cryptote.hhc.h \
	cryptote.hhk.h \
	cryptote.hhp.h \
	cryptote.hhp.cached.h \
	cryptote_contents.html.h \
	cryptote_introduction.html.h \
	cryptote_aboutencryption.html.h \
	cryptote_features.html.h

EXTRA_DIST = \
	style.css \
	back.gif \
	contents.gif \
	forward.gif \
	up.gif \
	cryptote.hhc \
	cryptote.hhk \
	cryptote.hhp \
	cryptote.hhp.cached \
	cryptote_contents.html \
	cryptote_introduction.html \
	cryptote_aboutencryption.html \
	cryptote_features.html
