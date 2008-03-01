// $Id$

#include "../crc32.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <iostream>

#include <gcrypt.h>

uint32_t swap_uint32(uint32_t w)
{
    const uint8_t* u = (uint8_t*)&w;
    return (u[0] << 24) | (u[1] << 16) | (u[2] << 8) | u[3];
}

int main()
{
    std::cout << "Testing CRC32 implementation against libgcrypt...";
    std::cout.flush();

    // Create libgcrypt CRC32 context

    gcry_md_hd_t gctx;

    gcry_error_t gcryerr = gcry_md_open(&gctx, GCRY_MD_CRC32, 0);
    if (gcryerr != 0) return -1;

    assert(gcry_md_get_algo_dlen(GCRY_MD_CRC32) == 4);

    // Hash random data using both methods

    srand( time(NULL) );

    uint8_t data[65536];

    for (unsigned int ri = 0; ri < 1000; ++ri)
    {
	size_t len = (rand() % 65536);

	// create random block
	for (size_t i = 0; i < len; ++i) {
	    data[i] = (uint8_t)rand();
	}

	// run through libgcrypt
	gcry_md_reset(gctx);
	gcry_md_write(gctx, data, len);
	gcry_md_final(gctx);

	// get results
	const uint32_t result1 = Enctain::crc32(data, len);
	const uint8_t* result2 = gcry_md_read(gctx, 0);

	// compare digests, for some reason libgcrypt's digest is swapped.
	assert(swap_uint32(result1) == *(uint32_t*)result2);
    }

    gcry_md_close(gctx);

    std::cout << "OK\n";

    return 0;
}
