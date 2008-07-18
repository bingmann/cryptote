# $Id$

SUBDIRS = . testsuite speedtest

noinst_LIBRARIES = libenctain.a

libenctain_a_SOURCES = bytebuff.h \
	mycrc32.cpp mycrc32.h \
	mysha256.cpp mysha256.h \
	myserpent.cpp myserpent.h \
	enctain.cpp enctain.h \
	\
	botan-1.6/botan/aes.h \
	botan-1.6/botan/allocate.h \
	botan-1.6/botan/base.h \
	botan-1.6/botan/base64.h \
	botan-1.6/botan/basefilt.h \
	botan-1.6/botan/bit_ops.h \
	botan-1.6/botan/botan.h \
	botan-1.6/botan/buf_es.h \
	botan-1.6/botan/buf_filt.h \
	botan-1.6/botan/build.h \
	botan-1.6/botan/bzip2.h \
	botan-1.6/botan/cbc.h \
	botan-1.6/botan/charset.h \
	botan-1.6/botan/config.h \
	botan-1.6/botan/crc32.h \
	botan-1.6/botan/data_snk.h \
	botan-1.6/botan/data_src.h \
	botan-1.6/botan/def_char.h \
	botan-1.6/botan/defalloc.h \
	botan-1.6/botan/des.h \
	botan-1.6/botan/ecb.h \
	botan-1.6/botan/eng_def.h \
	botan-1.6/botan/engine.h \
	botan-1.6/botan/enums.h \
	botan-1.6/botan/es_capi.h \
	botan-1.6/botan/es_egd.h \
	botan-1.6/botan/es_file.h \
	botan-1.6/botan/es_ftw.h \
	botan-1.6/botan/es_unix.h \
	botan-1.6/botan/es_win32.h \
	botan-1.6/botan/exceptn.h \
	botan-1.6/botan/filter.h \
	botan-1.6/botan/filters.h \
	botan-1.6/botan/fips140.h \
	botan-1.6/botan/hex.h \
	botan-1.6/botan/hmac.h \
	botan-1.6/botan/init.h \
	botan-1.6/botan/libstate.h \
	botan-1.6/botan/look_add.h \
	botan-1.6/botan/lookup.h \
	botan-1.6/botan/md5.h \
	botan-1.6/botan/mdx_hash.h \
	botan-1.6/botan/mem_ops.h \
	botan-1.6/botan/mem_pool.h \
	botan-1.6/botan/mmap_mem.h \
	botan-1.6/botan/mode_pad.h \
	botan-1.6/botan/modebase.h \
	botan-1.6/botan/modules.h \
	botan-1.6/botan/mutex.h \
	botan-1.6/botan/mux_pthr.h \
	botan-1.6/botan/mux_win32.h \
	botan-1.6/botan/out_buf.h \
	botan-1.6/botan/parsing.h \
	botan-1.6/botan/pipe.h \
	botan-1.6/botan/pkcs5.h \
	botan-1.6/botan/randpool.h \
	botan-1.6/botan/rng.h \
	botan-1.6/botan/s2k.h \
	botan-1.6/botan/secmem.h \
	botan-1.6/botan/secqueue.h \
	botan-1.6/botan/serpent.h \
	botan-1.6/botan/sha160.h \
	botan-1.6/botan/sha256.h \
	botan-1.6/botan/sha_64.h \
	botan-1.6/botan/stl_util.h \
	botan-1.6/botan/symkey.h \
	botan-1.6/botan/timers.h \
	botan-1.6/botan/tm_hard.h \
	botan-1.6/botan/tm_posix.h \
	botan-1.6/botan/tm_unix.h \
	botan-1.6/botan/tm_win32.h \
	botan-1.6/botan/twofish.h \
	botan-1.6/botan/types.h \
	botan-1.6/botan/ui.h \
	botan-1.6/botan/unix_cmd.h \
	botan-1.6/botan/util.h \
	botan-1.6/botan/version.h \
	botan-1.6/botan/x931_rng.h \
	botan-1.6/botan/zlib.h \
	\
	botan-1.6/src/aes.cpp \
	botan-1.6/src/aes_tab.cpp \
	botan-1.6/src/base.cpp \
	botan-1.6/src/base64.cpp \
	botan-1.6/src/basefilt.cpp \
	botan-1.6/src/bit_ops.cpp \
	botan-1.6/src/buf_es.cpp \
	botan-1.6/src/buf_filt.cpp \
	botan-1.6/src/cbc.cpp \
	botan-1.6/src/charset.cpp \
	botan-1.6/src/config.cpp \
	botan-1.6/src/crc32.cpp \
	botan-1.6/src/data_snk.cpp \
	botan-1.6/src/data_src.cpp \
	botan-1.6/src/def_alg.cpp \
	botan-1.6/src/def_char.cpp \
	botan-1.6/src/def_mode.cpp \
	botan-1.6/src/defalloc.cpp \
	botan-1.6/src/des.cpp \
	botan-1.6/src/des_tab.cpp \
	botan-1.6/src/ecb.cpp \
	botan-1.6/src/eng_base.cpp \
	botan-1.6/src/engine.cpp \
	botan-1.6/src/es_file.cpp \
	botan-1.6/src/exceptn.cpp \
	botan-1.6/src/filter.cpp \
	botan-1.6/src/filters.cpp \
	botan-1.6/src/fips140.cpp \
	botan-1.6/src/get_algo.cpp \
	botan-1.6/src/get_enc.cpp \
	botan-1.6/src/hex.cpp \
	botan-1.6/src/hmac.cpp \
	botan-1.6/src/inifile.cpp \
	botan-1.6/src/init_def.cpp \
	botan-1.6/src/init_opt.cpp \
	botan-1.6/src/libstate.cpp \
	botan-1.6/src/md5.cpp \
	botan-1.6/src/mdx_hash.cpp \
	botan-1.6/src/mem_pool.cpp \
	botan-1.6/src/mlock.cpp \
	botan-1.6/src/mode_pad.cpp \
	botan-1.6/src/modebase.cpp \
	botan-1.6/src/modules.cpp \
	botan-1.6/src/mutex.cpp \
	botan-1.6/src/out_buf.cpp \
	botan-1.6/src/parsing.cpp \
	botan-1.6/src/pipe.cpp \
	botan-1.6/src/pipe_io.cpp \
	botan-1.6/src/pipe_rw.cpp \
	botan-1.6/src/pkcs5.cpp \
	botan-1.6/src/policy.cpp \
	botan-1.6/src/randpool.cpp \
	botan-1.6/src/rng.cpp \
	botan-1.6/src/s2k.cpp \
	botan-1.6/src/secqueue.cpp \
	botan-1.6/src/serpent.cpp \
	botan-1.6/src/sha160.cpp \
	botan-1.6/src/sha256.cpp \
	botan-1.6/src/sha_64.cpp \
	botan-1.6/src/symkey.cpp \
	botan-1.6/src/timers.cpp \
	botan-1.6/src/two_tab.cpp \
	botan-1.6/src/twofish.cpp \
	botan-1.6/src/ui.cpp \
	botan-1.6/src/util.cpp \
	botan-1.6/src/x931_rng.cpp \
	\
	botan-1.6/modules/comp_bzip2/bzip2.cpp \
	botan-1.6/modules/comp_zlib/zlib.cpp

AM_CXXFLAGS = -W -Wall -I$(srcdir)/botan-1.6/ $(ZLIB_CFLAGS) $(BZLIB_CFLAGS)

EXTRA_DIST = format.html
