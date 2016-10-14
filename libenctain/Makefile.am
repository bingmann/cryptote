################################################################################
# libenctain/Makefile.am
#
# Part of CryptoTE, see http://panthema.net/2007/cryptote
#
# Copyright (C) 2008-2014 Timo Bingmann <tb@panthema.net>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA 02111-1307 USA
################################################################################

SUBDIRS = . testsuite

AM_CXXFLAGS = -W -Wall $(ZLIB_CFLAGS) $(BZLIB_CFLAGS)

noinst_LIBRARIES = libenctain.a

libenctain_a_SOURCES = \
	bytebuff.h encios.h \
	enctain.cpp enctain.h \
	initrng.cpp \
	\
	botan-1.6/include/aes.h \
	botan-1.6/include/allocate.h \
	botan-1.6/include/base.h \
	botan-1.6/include/base64.h \
	botan-1.6/include/basefilt.h \
	botan-1.6/include/bit_ops.h \
	botan-1.6/include/botan.h \
	botan-1.6/include/buf_es.h \
	botan-1.6/include/buf_filt.h \
	botan-1.6/include/build.h \
	botan-1.6/include/bzip2.h \
	botan-1.6/include/cbc.h \
	botan-1.6/include/charset.h \
	botan-1.6/include/config.h \
	botan-1.6/include/crc32.h \
	botan-1.6/include/data_snk.h \
	botan-1.6/include/data_src.h \
	botan-1.6/include/def_char.h \
	botan-1.6/include/defalloc.h \
	botan-1.6/include/des.h \
	botan-1.6/include/ecb.h \
	botan-1.6/include/eng_def.h \
	botan-1.6/include/engine.h \
	botan-1.6/include/enums.h \
	botan-1.6/include/es_capi.h \
	botan-1.6/include/es_egd.h \
	botan-1.6/include/es_file.h \
	botan-1.6/include/es_ftw.h \
	botan-1.6/include/es_unix.h \
	botan-1.6/include/es_win32.h \
	botan-1.6/include/exceptn.h \
	botan-1.6/include/filter.h \
	botan-1.6/include/filters.h \
	botan-1.6/include/fips140.h \
	botan-1.6/include/hex.h \
	botan-1.6/include/hmac.h \
	botan-1.6/include/init.h \
	botan-1.6/include/libstate.h \
	botan-1.6/include/look_add.h \
	botan-1.6/include/lookup.h \
	botan-1.6/include/md5.h \
	botan-1.6/include/mdx_hash.h \
	botan-1.6/include/mem_ops.h \
	botan-1.6/include/mem_pool.h \
	botan-1.6/include/mmap_mem.h \
	botan-1.6/include/mode_pad.h \
	botan-1.6/include/modebase.h \
	botan-1.6/include/modules.h \
	botan-1.6/include/mutex.h \
	botan-1.6/include/mux_pthr.h \
	botan-1.6/include/mux_win32.h \
	botan-1.6/include/out_buf.h \
	botan-1.6/include/parsing.h \
	botan-1.6/include/pipe.h \
	botan-1.6/include/pkcs5.h \
	botan-1.6/include/randpool.h \
	botan-1.6/include/rng.h \
	botan-1.6/include/s2k.h \
	botan-1.6/include/secmem.h \
	botan-1.6/include/secqueue.h \
	botan-1.6/include/serpent.h \
	botan-1.6/include/sha160.h \
	botan-1.6/include/sha256.h \
	botan-1.6/include/stl_util.h \
	botan-1.6/include/symkey.h \
	botan-1.6/include/timers.h \
	botan-1.6/include/tm_hard.h \
	botan-1.6/include/tm_posix.h \
	botan-1.6/include/tm_unix.h \
	botan-1.6/include/tm_win32.h \
	botan-1.6/include/types.h \
	botan-1.6/include/ui.h \
	botan-1.6/include/unix_cmd.h \
	botan-1.6/include/util.h \
	botan-1.6/include/version.h \
	botan-1.6/include/x931_rng.h \
	botan-1.6/include/zlib.h \
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
	botan-1.6/src/symkey.cpp \
	botan-1.6/src/timers.cpp \
	botan-1.6/src/ui.cpp \
	botan-1.6/src/util.cpp \
	botan-1.6/src/x931_rng.cpp \
	\
	botan-1.6/modules/comp_bzip2/bzip2.cpp \
	botan-1.6/modules/comp_zlib/zlib.cpp

if ON_WIN32

libenctain_a_SOURCES += \
	botan-1.6/modules/es_capi/es_capi.cpp \
	botan-1.6/modules/es_win32/es_win32.cpp \
	botan-1.6/modules/ml_win32/mlock_win32.cpp \
	botan-1.6/modules/mux_win32/mux_win32.cpp \
	botan-1.6/modules/tm_win32/tm_win32.cpp

AM_CXXFLAGS += -DHAVE_ON_WIN32

else

libenctain_a_SOURCES += \
	botan-1.6/modules/alloc_mmap/mmap_mem.cpp \
	botan-1.6/modules/es_egd/es_egd.cpp \
	botan-1.6/modules/es_ftw/es_ftw.cpp \
	botan-1.6/modules/es_unix/es_unix.cpp \
	botan-1.6/modules/es_unix/unix_cmd.cpp \
	botan-1.6/modules/es_unix/unix_src.cpp \
	botan-1.6/modules/ml_unix/mlock_unix.cpp \
	botan-1.6/modules/mux_pthr/mux_pthr.cpp \
	botan-1.6/modules/tm_hard/tm_hard.cpp \
	botan-1.6/modules/tm_posix/tm_posix.cpp \
	botan-1.6/modules/tm_unix/tm_unix.cpp

endif

EXTRA_DIST = \
	format.html \
	format.pdf \
	botan-1.6/README \
	botan-1.6/license.txt \
	botan-1.6/modules/alloc_mmap/modinfo.txt \
	botan-1.6/modules/comp_bzip2/modinfo.txt \
	botan-1.6/modules/comp_zlib/modinfo.txt \
	botan-1.6/modules/es_capi/modinfo.txt \
	botan-1.6/modules/es_egd/modinfo.txt \
	botan-1.6/modules/es_ftw/modinfo.txt \
	botan-1.6/modules/es_unix/modinfo.txt \
	botan-1.6/modules/es_win32/modinfo.txt \
	botan-1.6/modules/ml_unix/modinfo.txt \
	botan-1.6/modules/ml_win32/modinfo.txt \
	botan-1.6/modules/mux_pthr/modinfo.txt \
	botan-1.6/modules/mux_win32/modinfo.txt \
	botan-1.6/modules/tm_hard/modinfo.txt \
	botan-1.6/modules/tm_posix/modinfo.txt \
	botan-1.6/modules/tm_unix/modinfo.txt \
	botan-1.6/modules/tm_win32/modinfo.txt
