# $Id$
# Based on wxWidgets-2.8.7/locale/Makefile

LINGUAS = de

# the programs we use (TODO: use configure to detect them)
MSGFMT=msgfmt --verbose
MSGMERGE=msgmerge --verbose
XGETTEXT=xgettext

# common xgettext args: C++ syntax, joining, use the specified macro names as markers
XGETTEXT_ARGS=-CF -k_ -kwxPLURAL:1,2 -kwxTRANSLATE

# implicit rule: generate binary message catalogs
%.mo: %.po
	$(MSGFMT) -c -o $@ $<

%.h: %.mo
	../art/file2h -p "locale-" $< > $@

# PO files must be updated from cryptote.pot to include new translations
%.po: cryptote.pot
	if [ -f $@ ]; then $(MSGMERGE) $@ cryptote.pot > $@.new && mv $@.new $@; else cp cryptote.pot $@; fi

cryptote.pot:
	$(XGETTEXT) $(XGETTEXT_ARGS) -o cryptote.pot ../pwgen/*.h ../pwgen/*.cpp
	$(XGETTEXT) $(XGETTEXT_ARGS) -o cryptote.pot ../cryptote/*.h ../cryptote/*.cpp

force-update:
	$(RM) cryptote.pot

allpo: force-update
	@for t in $(LINGUAS); do $(MAKE) $$t.po; done

allmo:
	@for t in $(LINGUAS); do $(MAKE) $$t.mo; done

allh:
	@for t in $(LINGUAS); do $(MAKE) $$t.h; done

all-local: allmo allh

# print out the percentage of the translated strings
stats:
	@for i in $(LINGUAS); do \
		x=`$(MSGFMT) -o /dev/null "$$i.po" 2>&1 | sed -e 's/[,\.]//g' \
			-e 's/\([0-9]\+\) translated messages\?/TR=\1/' \
			-e 's/\([0-9]\+\) fuzzy translations\?/FZ=\1/' \
			-e 's/\([0-9]\+\) untranslated messages\?/UT=\1/'`; \
		TR=0 FZ=0 UT=0; \
		eval $$x; \
		TOTAL=`expr $$TR + $$FZ + $$UT`; \
		echo "$$i.po `expr 100 "*" $$TR / $$TOTAL`% of $$TOTAL strings"; \
	done

links: allmo
	@for t in $(LINGUAS); do [ ! -d $$t ] && mkdir $$t; ln -sf ../$$t.mo $$t/cryptote.mo; done

.PHONY: allpo allmo force-update stats links
