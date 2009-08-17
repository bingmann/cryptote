# $Id$
# Based on wxWidgets-2.8.7/locale/Makefile

LINGUAS = de

# the gettext programs are detected by configure
if GOT_GETTEXT

# implicit rule: generate binary message catalogs
%.mo: %.po
	$(MSGFMT) --verbose -c -o $@ $<

%.h: %.mo
	../../art/file2h -c -p "locale-wxstd-" $< > $@

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

endif

.PHONY: allmo allh force-update stats

EXTRA_DIST = \
	de.po de.mo de.h
