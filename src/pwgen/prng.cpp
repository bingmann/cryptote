// $Id$

#include "prng.h"

#include <assert.h>
#include <stdio.h>

#include <wx/wx.h>

PRNG::PRNG()
    : len_(0), used_(0)
{
}

void PRNG::generate()
{
    used_ = 0;

#ifdef __WIN32__
    len_ = generate_win32();
    if (len_ != 0) return;
#endif

#ifdef __UNIX__
    len_ = generate_unix();
    if (len_ != 0) return;
#endif

    len_ = generate_clock();
    if (len_ != 0) return;

    assert(0);
    return;
}

unsigned char PRNG::get_byte()
{
    if (used_ + 1 >= len_) generate();

    return data_[used_++];
}

unsigned int PRNG::get_int32()
{
    if (used_ + sizeof(unsigned int) >= len_) generate();

    unsigned int v = *(unsigned int*)(data_ + used_);
    used_ += sizeof(unsigned int);
    return v;
}

unsigned int PRNG::get(unsigned int limit)
{
    if (limit < 256)
	return get_byte() % limit;
    else
	return get_int32() % limit;
}

// *** Random number generators are based on libtomcrypt *** //

// on *NIX read /dev/random
size_t PRNG::generate_unix()
{
    FILE* f = fopen("/dev/urandom", "rb");
    if (f == NULL)
    {
	f = fopen("/dev/random", "rb");

	if (f == NULL) {
	    return 0;
	}
    }

    /* disable buffering */
    if (setvbuf(f, NULL, _IONBF, 0) != 0) {
	fclose(f);
	return 0;
    }

    int x = (int)fread(data_, 1, sizeof(data_), f);
    fclose(f);

    return x;
}

// On Windows: try the Microsoft CryptoServiceProvider
#if defined(__WIN32__) || defined(__WXWINCE__)

#ifndef _WIN32_WINNT
   #define _WIN32_WINNT 0x0400
#endif
#ifdef WINCE
   #define UNDER_CE
   #define ARM
#endif

#include <windows.h>
#include <wincrypt.h>

size_t PRNG::generate_win32()
{
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
			     (CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET)) &&
	!CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
			      CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
    {
	return 0;
    }

    if (CryptGenRandom(hProv, sizeof(data_), data_) == TRUE)
    {
	CryptReleaseContext(hProv, 0);
	return sizeof(data_);
    }
    else
    {
	CryptReleaseContext(hProv, 0);
	return 0;
    }
}

#else

size_t PRNG::generate_win32()
{
    return 0;
}

#endif /* WIN32 */

size_t PRNG::generate_clock()
{
    clock_t t1;
    int acc, a, b;

    int bits = 8;
    acc = a = b = 0;
    unsigned int pos;

    for(pos = 0; pos < sizeof(data_); ++pos)
    {
	wxSafeYield();

	while (bits--) {
	    do {
		t1 = clock(); while (t1 == clock()) a ^= 1;
		t1 = clock(); while (t1 == clock()) b ^= 1;
	    } while (a == b);
	    acc = (acc << 1) | a;
	}

	data_[pos] = acc;
	acc  = 0;
	bits = 8;
    }

    return pos;
}
