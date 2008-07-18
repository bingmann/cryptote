// $Id$

#include "../mysha256.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <gcrypt.h>

#include <iostream>

int main()
{
    std::cout << "Testing SHA256 implementation against libgcrypt...";
    std::cout.flush();

    // Create SHA256 context

    Enctain::internal::SHA256 ctx1;

    // Create libgcrypt SHA256 context

    gcry_md_hd_t ctx2;

    gcry_error_t gcryerr = gcry_md_open(&ctx2, GCRY_MD_SHA256, 0);
    if (gcryerr != 0) return -1;

    assert(gcry_md_get_algo_dlen(GCRY_MD_SHA256) == 32);

    // Hash random data using both contexts.

    srand( time(NULL) );

    uint8_t data[65536];

    for (unsigned int ri = 0; ri < 1000; ++ri)
    {
	size_t len = rand() % 65536;

	// create random block
	for (size_t i = 0; i < len; ++i) {
	    data[i] = (uint8_t)rand();
	}

	// reset SHA contexts
	ctx1.reset();
	gcry_md_reset(ctx2);

	// hash data
	ctx1.update(data, len);

	gcry_md_write(ctx2, data, len);
	gcry_md_final(ctx2);

	// get results
	const uint8_t* result1 = ctx1.final();
	const uint8_t* result2 = gcry_md_read(ctx2, 0);

	// compare digests
	for (unsigned int i = 0; i < 32; ++i) {
	    assert(result1[i] == result2[i]);
	}
    }

    gcry_md_close(ctx2);

    std::cout << "OK\n";

    return 0;
}
