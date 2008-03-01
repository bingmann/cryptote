// $Id$

/*
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Last update: 02/02/2007
 * Issue date:  04/30/2005
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>

#include "sha256.h"

namespace Enctain {

#define SHFR(x, n)    (x >> n)
#define ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define CH(x, y, z)  ((x & y) ^ (~x & z))
#define MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))

#define SHA256_F1(x) (ROTR(x,  2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SHA256_F2(x) (ROTR(x,  6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SHA256_F3(x) (ROTR(x,  7) ^ ROTR(x, 18) ^ SHFR(x,  3))
#define SHA256_F4(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHFR(x, 10))

#define UNPACK32(x, str)                      \
{                                             \
    *((str) + 3) = (uint8_t) ((x)      );     \
    *((str) + 2) = (uint8_t) ((x) >>  8);     \
    *((str) + 1) = (uint8_t) ((x) >> 16);     \
    *((str) + 0) = (uint8_t) ((x) >> 24);     \
}

#define PACK32(str, x)                        \
{                                             \
    *(x) =   ((uint32_t) *((str) + 3)      )  \
           | ((uint32_t) *((str) + 2) <<  8)  \
           | ((uint32_t) *((str) + 1) << 16)  \
           | ((uint32_t) *((str) + 0) << 24); \
}

#define SWAP32(x)                             \
{                                             \
    PACK32( (uint8_t*)x, x)		      \
}

/* Macros used for loops unrolling */

#define SHA256_SCR(i)                         \
{                                             \
    w[i] =  SHA256_F4(w[i -  2]) + w[i -  7]  \
          + SHA256_F3(w[i - 15]) + w[i - 16]; \
}

#define SHA256_EXP(a, b, c, d, e, f, g, h, j)               \
{                                                           \
    t1 = wv[h] + SHA256_F2(wv[e]) + CH(wv[e], wv[f], wv[g]) \
         + sha256_k[j] + w[j];                              \
    t2 = SHA256_F1(wv[a]) + MAJ(wv[a], wv[b], wv[c]);       \
    wv[d] += t1;                                            \
    wv[h] = t1 + t2;                                        \
}

static const uint32_t sha256_k[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

/* SHA-256 functions */

SHA256::SHA256()
{
    h[0] = 0x6a09e667; h[1] = 0xbb67ae85;
    h[2] = 0x3c6ef372; h[3] = 0xa54ff53a;
    h[4] = 0x510e527f; h[5] = 0x9b05688c;
    h[6] = 0x1f83d9ab; h[7] = 0x5be0cd19;

    total = 0;
}

void SHA256::reset()
{
    h[0] = 0x6a09e667; h[1] = 0xbb67ae85;
    h[2] = 0x3c6ef372; h[3] = 0xa54ff53a;
    h[4] = 0x510e527f; h[5] = 0x9b05688c;
    h[6] = 0x1f83d9ab; h[7] = 0x5be0cd19;

    total = 0;
}

void SHA256::process(const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t wv[8];
    uint32_t t1, t2;

    PACK32(&block[ 0], &w[ 0]); PACK32(&block[ 4], &w[ 1]);
    PACK32(&block[ 8], &w[ 2]); PACK32(&block[12], &w[ 3]);
    PACK32(&block[16], &w[ 4]); PACK32(&block[20], &w[ 5]);
    PACK32(&block[24], &w[ 6]); PACK32(&block[28], &w[ 7]);
    PACK32(&block[32], &w[ 8]); PACK32(&block[36], &w[ 9]);
    PACK32(&block[40], &w[10]); PACK32(&block[44], &w[11]);
    PACK32(&block[48], &w[12]); PACK32(&block[52], &w[13]);
    PACK32(&block[56], &w[14]); PACK32(&block[60], &w[15]);

    SHA256_SCR(16); SHA256_SCR(17); SHA256_SCR(18); SHA256_SCR(19);
    SHA256_SCR(20); SHA256_SCR(21); SHA256_SCR(22); SHA256_SCR(23);
    SHA256_SCR(24); SHA256_SCR(25); SHA256_SCR(26); SHA256_SCR(27);
    SHA256_SCR(28); SHA256_SCR(29); SHA256_SCR(30); SHA256_SCR(31);
    SHA256_SCR(32); SHA256_SCR(33); SHA256_SCR(34); SHA256_SCR(35);
    SHA256_SCR(36); SHA256_SCR(37); SHA256_SCR(38); SHA256_SCR(39);
    SHA256_SCR(40); SHA256_SCR(41); SHA256_SCR(42); SHA256_SCR(43);
    SHA256_SCR(44); SHA256_SCR(45); SHA256_SCR(46); SHA256_SCR(47);
    SHA256_SCR(48); SHA256_SCR(49); SHA256_SCR(50); SHA256_SCR(51);
    SHA256_SCR(52); SHA256_SCR(53); SHA256_SCR(54); SHA256_SCR(55);
    SHA256_SCR(56); SHA256_SCR(57); SHA256_SCR(58); SHA256_SCR(59);
    SHA256_SCR(60); SHA256_SCR(61); SHA256_SCR(62); SHA256_SCR(63);

    wv[0] = h[0]; wv[1] = h[1];
    wv[2] = h[2]; wv[3] = h[3];
    wv[4] = h[4]; wv[5] = h[5];
    wv[6] = h[6]; wv[7] = h[7];

    SHA256_EXP(0,1,2,3,4,5,6,7, 0); SHA256_EXP(7,0,1,2,3,4,5,6, 1);
    SHA256_EXP(6,7,0,1,2,3,4,5, 2); SHA256_EXP(5,6,7,0,1,2,3,4, 3);
    SHA256_EXP(4,5,6,7,0,1,2,3, 4); SHA256_EXP(3,4,5,6,7,0,1,2, 5);
    SHA256_EXP(2,3,4,5,6,7,0,1, 6); SHA256_EXP(1,2,3,4,5,6,7,0, 7);
    SHA256_EXP(0,1,2,3,4,5,6,7, 8); SHA256_EXP(7,0,1,2,3,4,5,6, 9);
    SHA256_EXP(6,7,0,1,2,3,4,5,10); SHA256_EXP(5,6,7,0,1,2,3,4,11);
    SHA256_EXP(4,5,6,7,0,1,2,3,12); SHA256_EXP(3,4,5,6,7,0,1,2,13);
    SHA256_EXP(2,3,4,5,6,7,0,1,14); SHA256_EXP(1,2,3,4,5,6,7,0,15);
    SHA256_EXP(0,1,2,3,4,5,6,7,16); SHA256_EXP(7,0,1,2,3,4,5,6,17);
    SHA256_EXP(6,7,0,1,2,3,4,5,18); SHA256_EXP(5,6,7,0,1,2,3,4,19);
    SHA256_EXP(4,5,6,7,0,1,2,3,20); SHA256_EXP(3,4,5,6,7,0,1,2,21);
    SHA256_EXP(2,3,4,5,6,7,0,1,22); SHA256_EXP(1,2,3,4,5,6,7,0,23);
    SHA256_EXP(0,1,2,3,4,5,6,7,24); SHA256_EXP(7,0,1,2,3,4,5,6,25);
    SHA256_EXP(6,7,0,1,2,3,4,5,26); SHA256_EXP(5,6,7,0,1,2,3,4,27);
    SHA256_EXP(4,5,6,7,0,1,2,3,28); SHA256_EXP(3,4,5,6,7,0,1,2,29);
    SHA256_EXP(2,3,4,5,6,7,0,1,30); SHA256_EXP(1,2,3,4,5,6,7,0,31);
    SHA256_EXP(0,1,2,3,4,5,6,7,32); SHA256_EXP(7,0,1,2,3,4,5,6,33);
    SHA256_EXP(6,7,0,1,2,3,4,5,34); SHA256_EXP(5,6,7,0,1,2,3,4,35);
    SHA256_EXP(4,5,6,7,0,1,2,3,36); SHA256_EXP(3,4,5,6,7,0,1,2,37);
    SHA256_EXP(2,3,4,5,6,7,0,1,38); SHA256_EXP(1,2,3,4,5,6,7,0,39);
    SHA256_EXP(0,1,2,3,4,5,6,7,40); SHA256_EXP(7,0,1,2,3,4,5,6,41);
    SHA256_EXP(6,7,0,1,2,3,4,5,42); SHA256_EXP(5,6,7,0,1,2,3,4,43);
    SHA256_EXP(4,5,6,7,0,1,2,3,44); SHA256_EXP(3,4,5,6,7,0,1,2,45);
    SHA256_EXP(2,3,4,5,6,7,0,1,46); SHA256_EXP(1,2,3,4,5,6,7,0,47);
    SHA256_EXP(0,1,2,3,4,5,6,7,48); SHA256_EXP(7,0,1,2,3,4,5,6,49);
    SHA256_EXP(6,7,0,1,2,3,4,5,50); SHA256_EXP(5,6,7,0,1,2,3,4,51);
    SHA256_EXP(4,5,6,7,0,1,2,3,52); SHA256_EXP(3,4,5,6,7,0,1,2,53);
    SHA256_EXP(2,3,4,5,6,7,0,1,54); SHA256_EXP(1,2,3,4,5,6,7,0,55);
    SHA256_EXP(0,1,2,3,4,5,6,7,56); SHA256_EXP(7,0,1,2,3,4,5,6,57);
    SHA256_EXP(6,7,0,1,2,3,4,5,58); SHA256_EXP(5,6,7,0,1,2,3,4,59);
    SHA256_EXP(4,5,6,7,0,1,2,3,60); SHA256_EXP(3,4,5,6,7,0,1,2,61);
    SHA256_EXP(2,3,4,5,6,7,0,1,62); SHA256_EXP(1,2,3,4,5,6,7,0,63);

    h[0] += wv[0]; h[1] += wv[1];
    h[2] += wv[2]; h[3] += wv[3];
    h[4] += wv[4]; h[5] += wv[5];
    h[6] += wv[6]; h[7] += wv[7];
}

void SHA256::update(const unsigned char *message, unsigned int msglen)
{
    if (msglen == 0) return;

    unsigned int left = total & 0x3F;
    unsigned int fill = 64 - left;

    total += msglen;

    // Still bytes in the block buffer left and the message has enough to fill
    // the block
    if (left && msglen >= fill)
    {
	memcpy(buffer + left, message, fill);

	process(buffer);
	msglen -= fill;
	message += fill;
	left = 0;
    }

    // Process complete blocks
    while (msglen >= 64)
    {
	process(message);
	msglen -= 64;
	message += 64;
    }

    // Bytes remaining in message
    if (msglen > 0) {
	memcpy(buffer + left, message, msglen);
    }
}

static const uint8_t padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint8_t* SHA256::final()
{
    if (total != 0xFFFFFFFF)
    {
	// append size of input
	uint32_t high = ( total >> 29 );
	uint32_t low  = ( total <<  3 );

	uint8_t msglen[8];
	UNPACK32(high, msglen+0);
	UNPACK32(low,  msglen+4);

	// but before adding size of input pad so that one block is filled.
	unsigned int last = total & 0x3F;
	unsigned int padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	update(padding, padn);
	update(msglen, 8);

	// assert( (total & 0x3F) == 0 );

	total = 0xFFFFFFFF;

	SWAP32(&h[0]);
	SWAP32(&h[1]);
	SWAP32(&h[2]);
	SWAP32(&h[3]);
	SWAP32(&h[4]);
	SWAP32(&h[5]);
	SWAP32(&h[6]);
	SWAP32(&h[7]);
    }

    return (const uint8_t*)h;
}

std::string SHA256::final_str()
{
    const uint8_t* dig = final();
    return std::string((const char*)(dig), 32);
}

std::string SHA256::digest(const std::string& data)
{
    SHA256 sha;
    sha.update(data);
    return sha.final_str();
}

#ifdef TEST_VECTORS

void sha256(const unsigned char *message, unsigned int len, unsigned char *digest)
{
    SHA256 ctx;

    ctx.update(message, len);
    const uint8_t* dig = ctx.final();
    memcpy(digest, dig, 32);
}

/* FIPS 180-2 Validation tests */

#include <stdio.h>
#include <stdlib.h>

void test(const char *vector, unsigned char *digest,
          unsigned int digest_size)
{
    unsigned char output[2 * digest_size + 1];
    int i;

    output[2 * digest_size] = '\0';

    for (i = 0; i < (int) digest_size ; i++) {
	sprintf((char *) output + 2 * i, "%02x", digest[i]);
    }

    printf("H: %s\n", output);
    if (strcmp((char *) vector, (char *) output)) {
        fprintf(stderr, "Test failed.\n");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    static const char *vectors[4][3] =
    { /* SHA-256 */
        {
	    "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
	    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
	    "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
        },
    };

    static const char message1[] = "abc";
    static const char message2a[] =
	"abcdbcdecdefdefgefghfghighijhi"
	"jkijkljklmklmnlmnomnopnopq";
    static const char message2b[] =
	"abcdefghbcdefghicdefghijdefghijkefghij"
	"klfghijklmghijklmnhijklmnoijklmnopjklm"
	"nopqklmnopqrlmnopqrsmnopqrstnopqrstu";

    unsigned char *message3;
    unsigned int message3_len = 1000000;
    unsigned char digest[32];

    message3 = (unsigned char*)malloc(message3_len);
    if (message3 == NULL) {
        fprintf(stderr, "Can't allocate memory\n");
        return -1;
    }
    memset(message3, 'a', message3_len);

    printf("SHA-2 FIPS 180-2 Validation tests\n\n");

    printf("SHA-256 Test vectors\n");

    sha256((unsigned char*)message1, strlen((char *) message1), digest);
    test(vectors[0][0], digest, 32);
    sha256((unsigned char*)message2a, strlen((char *) message2a), digest);
    test(vectors[0][1], digest, 32);
    sha256((unsigned char*)message3, message3_len, digest);
    test(vectors[0][2], digest, 32);
    printf("\n");

    printf("All tests passed.\n");

    return 0;
}

#endif /* TEST_VECTORS */

} // namespace Enctain
