// $Id$

#include "../serpent.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <iostream>

#include <gcrypt.h>

int main()
{
    const char* key = "BVksLhOTmqxETMvfnbhE3xxx2RLRI52H";
    const char* iv = "ABCDEFGHIJKLMNOP";

    std::cout << "Testing Serpent CBC implementation against libgcrypt...";
    std::cout.flush();

    // Create contexts

    Enctain::internal::SerpentCBC encctx1;

    encctx1.set_key((uint8_t*)key, 256);
    encctx1.set_cbciv((uint8_t*)iv);

    Enctain::internal::SerpentCBC decctx1;

    decctx1.set_key((uint8_t*)key, 256);
    decctx1.set_cbciv((uint8_t*)iv);

    // Create libgcrypt Serpent context

    gcry_cipher_hd_t encctx2;

    gcry_error_t gcryerr = gcry_cipher_open(&encctx2, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CBC, 0);
    if (gcryerr != 0) return -1;

    gcryerr = gcry_cipher_setkey(encctx2, (const uint8_t*)key, 32);
    if (gcryerr != 0) return -1;

    gcryerr = gcry_cipher_setiv(encctx2, (const uint8_t*)iv, 16);
    if (gcryerr != 0) return -1;

    gcry_cipher_hd_t decctx2;

    gcryerr = gcry_cipher_open(&decctx2, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_CBC, 0);
    if (gcryerr != 0) return -1;

    gcryerr = gcry_cipher_setkey(decctx2, (const uint8_t*)key, 32);
    if (gcryerr != 0) return -1;

    gcryerr = gcry_cipher_setiv(decctx2, (const uint8_t*)iv, 16);
    if (gcryerr != 0) return -1;

    // Encrypt and Decrypt random data using both contexts.

    srand( time(NULL) );

    const unsigned int datalen = 16 * 1024;

    uint8_t odata[datalen];
    uint8_t ctdata1[datalen], ctdata2[datalen];
    uint8_t ptdata1[datalen], ptdata2[datalen];

    for (unsigned int ri = 0; ri < 1000; ++ri)
    {
	// create random data buffer
	for (unsigned int i = 0; i < datalen; ++i) {
	    odata[i] = (uint8_t)rand();
	}

	// encrypt data
	encctx1.encrypt(odata, ctdata1, datalen);

	gcry_cipher_encrypt(encctx2, ctdata2, datalen, odata, datalen);

	// compare cipher texts
	for (unsigned int i = 0; i < datalen; ++i) {
	    assert(ctdata1[i] == ctdata2[i]);
	}

	// decrypt data
	decctx1.decrypt(ctdata1, ptdata1, datalen);

	gcry_cipher_decrypt(decctx2, ptdata2, datalen, ctdata2, datalen);
	
	// compare plain texts
	for (unsigned int i = 0; i < datalen; ++i) {
	    assert(odata[i] == ptdata1[i]);
	    assert(ptdata1[i] == ptdata2[i]);
	}
    }

    gcry_cipher_close(encctx2);
    gcry_cipher_close(decctx2);

    std::cout << "OK\n";

    return 0;
}
