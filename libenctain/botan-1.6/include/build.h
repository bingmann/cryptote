/*************************************************
* Build Configuration Header File                *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#ifndef BOTAN_BUILD_CONFIG_H__
#define BOTAN_BUILD_CONFIG_H__

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 6
#define BOTAN_VERSION_PATCH 4

#define BOTAN_MP_WORD_BITS 32
#define BOTAN_DEFAULT_BUFFER_SIZE 4096

#define BOTAN_KARAT_MUL_THRESHOLD 12
#define BOTAN_KARAT_SQR_THRESHOLD 12

#define BOTAN_EXT_COMPRESSOR_BZIP2
#define BOTAN_EXT_COMPRESSOR_ZLIB

#ifdef HAVE_ON_WIN32

#define BOTAN_EXT_ENTROPY_SRC_CAPI
#define BOTAN_EXT_ENTROPY_SRC_WIN32
#define BOTAN_EXT_MUTEX_WIN32
#define BOTAN_EXT_TIMER_WIN32

#else

#define BOTAN_EXT_ALLOC_MMAP
#define BOTAN_EXT_ENTROPY_SRC_EGD
#define BOTAN_EXT_ENTROPY_SRC_FTW
#define BOTAN_EXT_ENTROPY_SRC_UNIX
#define BOTAN_EXT_MUTEX_PTHREAD
#define BOTAN_EXT_TIMER_UNIX

#if defined(BOTAN_TARGET_ARCH_IS_IA32) || defined(BOTAN_TARGET_ARCH_IS_AMD64) || \
    defined(BOTAN_TARGET_ARCH_IS_PPC) || defined(BOTAN_TARGET_ARCH_IS_PPC64)
#define BOTAN_EXT_TIMER_HARDWARE
#endif

#if HAVE_DECL_CLOCK_GETTIME
#define BOTAN_EXT_TIMER_POSIX
#endif

#endif // HAVE_ON_WIN32

#endif
