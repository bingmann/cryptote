# $Id$

SUBDIRS = libstc libenctain src desktop

EXTRA_DIST = cryptote-win32.iss

if GCOV

clean-local:
	find -name "*.da" -o -name "*.gcov" -o -name "*.gcda" -o -name "*.gcno" | xargs rm || true

run-gcov: clean-gcov
	mkdir -p coverage/
	lcov --directory . --zerocounters
	-$(MAKE) check
	lcov --directory . --capture --output-file coverage/all.info
	lcov -e coverage/all.info "/tdata/*" -o coverage/testsuite.info
	genhtml -o coverage --num-spaces 8 coverage/testsuite.info --title "CryptoTE Testsuites" --prefix `pwd`

clean-gcov:
	find -name "*.gcda" | xargs rm || true

endif
