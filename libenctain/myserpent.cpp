// $Id$

/*************************************************
* Serpent Source File                            *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#include "myserpent.h"

#include <string.h>
#include <assert.h>

namespace Enctain {
namespace internal {

/*** Bits and Pieces from other Botan sources ***/

typedef unsigned char byte;
typedef unsigned short u16bit;
typedef unsigned int u32bit;

/*************************************************
* Word Rotation Functions                        *
*************************************************/
template<typename T> static inline T rotate_left(T input, u32bit rot)
{
    return static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
}

template<typename T> static inline T rotate_right(T input, u32bit rot)
{
    return static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
}

/*************************************************
* Byte to Word Conversions                       *
*************************************************/
static inline u32bit make_u32bit(byte i0, byte i1, byte i2, byte i3)
{
    return ((static_cast<u32bit>(i0) << 24) |
	    (static_cast<u32bit>(i1) << 16) |
	    (static_cast<u32bit>(i2) <<  8) |
	    (static_cast<u32bit>(i3)));
}

/*************************************************
* Byte Extraction Function                       *
*************************************************/
template<typename T> static inline byte get_byte(u32bit byte_num, T input)
{
    return (input >> ((sizeof(T)-1-(byte_num&(sizeof(T)-1))) << 3));
}

/*************************************************
* Endian-Specific Word Loading Operations        *
*************************************************/
template<typename T>
static inline T load_le(const byte in[], u32bit off)
{
    in += off * sizeof(T);
    T out = 0;
    for(u32bit j = 0; j != sizeof(T); j++)
	out = (out << 8) | in[sizeof(T)-1-j];
    return out;
}

template<>
static inline u32bit load_le<u32bit>(const byte in[], u32bit off)
{
    in += off * sizeof(u32bit);
    return make_u32bit(in[3], in[2], in[1], in[0]);
}

/*************************************************
* Endian-Specific Word Storing Operations        *
*************************************************/

static inline void store_le(u32bit in, byte out[4])
{
    out[0] = get_byte(3, in);
    out[1] = get_byte(2, in);
    out[2] = get_byte(1, in);
    out[3] = get_byte(0, in);
}

template<typename T>
static inline void store_le(byte out[], T a, T b, T c, T d)
{
    store_le(a, out + (0 * sizeof(T)));
    store_le(b, out + (1 * sizeof(T)));
    store_le(c, out + (2 * sizeof(T)));
    store_le(d, out + (3 * sizeof(T)));
}

// *** CBC Methods ***

static inline void cbc_copy(const uint8_t src[16], uint8_t dst[16])
{
    ((uint32_t*)dst)[0] = ((uint32_t*)src)[0];
    ((uint32_t*)dst)[1] = ((uint32_t*)src)[1];
    ((uint32_t*)dst)[2] = ((uint32_t*)src)[2];
    ((uint32_t*)dst)[3] = ((uint32_t*)src)[3];
}

static inline void cbc_xor(const uint8_t src[16], uint8_t dst[16])
{
    ((uint32_t*)dst)[0] ^= ((uint32_t*)src)[0];
    ((uint32_t*)dst)[1] ^= ((uint32_t*)src)[1];
    ((uint32_t*)dst)[2] ^= ((uint32_t*)src)[2];
    ((uint32_t*)dst)[3] ^= ((uint32_t*)src)[3];
}

/*** serpent.c ***/

/*************************************************
* Serpent Encryption S-Box 1                     *
*************************************************/
inline void SBoxE1(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T3 ^= T0; T4  = T1; T1 &= T3; T4 ^= T2; T1 ^= T0; T0 |= T3; T0 ^= T4;
    T4 ^= T3; T3 ^= T2; T2 |= T1; T2 ^= T4; T4 = ~T4; T4 |= T1; T1 ^= T3;
    T1 ^= T4; T3 |= T0; T1 ^= T3; T4 ^= T3;
    B0 = T1; B1 = T4; B2 = T2; B3 = T0;
}

/*************************************************
* Serpent Encryption S-Box 2                     *
*************************************************/
inline void SBoxE2(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T0 = ~T0; T2 = ~T2; T4  = T0; T0 &= T1; T2 ^= T0; T0 |= T3; T3 ^= T2;
    T1 ^= T0; T0 ^= T4; T4 |= T1; T1 ^= T3; T2 |= T0; T2 &= T4; T0 ^= T1;
    T1 &= T2; T1 ^= T0; T0 &= T2; T0 ^= T4;
    B0 = T2; B1 = T0; B2 = T3; B3 = T1;
}

/*************************************************
* Serpent Encryption S-Box 3                     *
*************************************************/
inline void SBoxE3(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T0; T0 &= T2; T0 ^= T3; T2 ^= T1; T2 ^= T0; T3 |= T4; T3 ^= T1;
    T4 ^= T2; T1  = T3; T3 |= T4; T3 ^= T0; T0 &= T1; T4 ^= T0; T1 ^= T3;
    T1 ^= T4; T4 = ~T4;
    B0 = T2; B1 = T3; B2 = T1; B3 = T4;
}

/*************************************************
* Serpent Encryption S-Box 4                     *
*************************************************/
inline void SBoxE4(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T0; T0 |= T3; T3 ^= T1; T1 &= T4; T4 ^= T2; T2 ^= T3; T3 &= T0;
    T4 |= T1; T3 ^= T4; T0 ^= T1; T4 &= T0; T1 ^= T3; T4 ^= T2; T1 |= T0;
    T1 ^= T2; T0 ^= T3; T2  = T1; T1 |= T3; T1 ^= T0;
    B0 = T1; B1 = T2; B2 = T3; B3 = T4;
}

/*************************************************
* Serpent Encryption S-Box 5                     *
*************************************************/
inline void SBoxE5(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T1 ^= T3; T3 = ~T3; T2 ^= T3; T3 ^= T0; T4  = T1; T1 &= T3; T1 ^= T2;
    T4 ^= T3; T0 ^= T4; T2 &= T4; T2 ^= T0; T0 &= T1; T3 ^= T0; T4 |= T1;
    T4 ^= T0; T0 |= T3; T0 ^= T2; T2 &= T3; T0 = ~T0; T4 ^= T2;
    B0 = T1; B1 = T4; B2 = T0; B3 = T3;
}

/*************************************************
* Serpent Encryption S-Box 6                     *
*************************************************/
inline void SBoxE6(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T0 ^= T1; T1 ^= T3; T3 = ~T3; T4  = T1; T1 &= T0; T2 ^= T3; T1 ^= T2;
    T2 |= T4; T4 ^= T3; T3 &= T1; T3 ^= T0; T4 ^= T1; T4 ^= T2; T2 ^= T0;
    T0 &= T3; T2 = ~T2; T0 ^= T4; T4 |= T3; T2 ^= T4;
    B0 = T1; B1 = T3; B2 = T0; B3 = T2;
}

/*************************************************
* Serpent Encryption S-Box 7                     *
*************************************************/
inline void SBoxE7(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T2 = ~T2; T4  = T3; T3 &= T0; T0 ^= T4; T3 ^= T2; T2 |= T4; T1 ^= T3;
    T2 ^= T0; T0 |= T1; T2 ^= T1; T4 ^= T0; T0 |= T3; T0 ^= T2; T4 ^= T3;
    T4 ^= T0; T3 = ~T3; T2 &= T4; T2 ^= T3;
    B0 = T0; B1 = T1; B2 = T4; B3 = T2;
}

/*************************************************
* Serpent Encryption S-Box 8                     *
*************************************************/
inline void SBoxE8(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T1; T1 |= T2; T1 ^= T3; T4 ^= T2; T2 ^= T1; T3 |= T4; T3 &= T0;
    T4 ^= T2; T3 ^= T1; T1 |= T4; T1 ^= T0; T0 |= T4; T0 ^= T2; T1 ^= T4;
    T2 ^= T1; T1 &= T0; T1 ^= T4; T2 = ~T2; T2 |= T0; T4 ^= T2;
    B0 = T4; B1 = T3; B2 = T1; B3 = T0;
}

/*************************************************
* Serpent Decryption S-Box 1                     *
*************************************************/
inline void SBoxD1(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T2 = ~T2; T4  = T1; T1 |= T0; T4 = ~T4; T1 ^= T2; T2 |= T4; T1 ^= T3;
    T0 ^= T4; T2 ^= T0; T0 &= T3; T4 ^= T0; T0 |= T1; T0 ^= T2; T3 ^= T4;
    T2 ^= T1; T3 ^= T0; T3 ^= T1; T2 &= T3; T4 ^= T2;
    B0 = T0; B1 = T4; B2 = T1; B3 = T3;
}

/*************************************************
* Serpent Decryption S-Box 2                     *
*************************************************/
inline void SBoxD2(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T1; T1 ^= T3; T3 &= T1; T4 ^= T2; T3 ^= T0; T0 |= T1; T2 ^= T3;
    T0 ^= T4; T0 |= T2; T1 ^= T3; T0 ^= T1; T1 |= T3; T1 ^= T0; T4 = ~T4;
    T4 ^= T1; T1 |= T0; T1 ^= T0; T1 |= T4; T3 ^= T1;
    B0 = T4; B1 = T0; B2 = T3; B3 = T2;
}

/*************************************************
* Serpent Decryption S-Box 3                     *
*************************************************/
inline void SBoxD3(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T2 ^= T3; T3 ^= T0; T4  = T3; T3 &= T2; T3 ^= T1; T1 |= T2; T1 ^= T4;
    T4 &= T3; T2 ^= T3; T4 &= T0; T4 ^= T2; T2 &= T1; T2 |= T0; T3 = ~T3;
    T2 ^= T3; T0 ^= T3; T0 &= T1; T3 ^= T4; T3 ^= T0;
    B0 = T1; B1 = T4; B2 = T2; B3 = T3;
}

/*************************************************
* Serpent Decryption S-Box 4                     *
*************************************************/
inline void SBoxD4(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T2; T2 ^= T1; T0 ^= T2; T4 &= T2; T4 ^= T0; T0 &= T1; T1 ^= T3;
    T3 |= T4; T2 ^= T3; T0 ^= T3; T1 ^= T4; T3 &= T2; T3 ^= T1; T1 ^= T0;
    T1 |= T2; T0 ^= T3; T1 ^= T4; T0 ^= T1;
    B0 = T2; B1 = T1; B2 = T3; B3 = T0;
}

/*************************************************
* Serpent Decryption S-Box 5                     *
*************************************************/
inline void SBoxD5(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T2; T2 &= T3; T2 ^= T1; T1 |= T3; T1 &= T0; T4 ^= T2; T4 ^= T1;
    T1 &= T2; T0 = ~T0; T3 ^= T4; T1 ^= T3; T3 &= T0; T3 ^= T2; T0 ^= T1;
    T2 &= T0; T3 ^= T0; T2 ^= T4; T2 |= T3; T3 ^= T0; T2 ^= T1;
    B0 = T0; B1 = T3; B2 = T2; B3 = T4;
}

/*************************************************
* Serpent Decryption S-Box 6                     *
*************************************************/
inline void SBoxD6(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T1 = ~T1; T4  = T3; T2 ^= T1; T3 |= T0; T3 ^= T2; T2 |= T1; T2 &= T0;
    T4 ^= T3; T2 ^= T4; T4 |= T0; T4 ^= T1; T1 &= T2; T1 ^= T3; T4 ^= T2;
    T3 &= T4; T4 ^= T1; T3 ^= T4; T4 = ~T4; T3 ^= T0;
    B0 = T1; B1 = T4; B2 = T3; B3 = T2;
}

/*************************************************
* Serpent Decryption S-Box 7                     *
*************************************************/
inline void SBoxD7(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T0 ^= T2; T4  = T2; T2 &= T0; T4 ^= T3; T2 = ~T2; T3 ^= T1; T2 ^= T3;
    T4 |= T0; T0 ^= T2; T3 ^= T4; T4 ^= T1; T1 &= T3; T1 ^= T0; T0 ^= T3;
    T0 |= T2; T3 ^= T1; T4 ^= T0;
    B0 = T1; B1 = T2; B2 = T4; B3 = T3;
}

/*************************************************
* Serpent Decryption S-Box 8                     *
*************************************************/
inline void SBoxD8(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    u32bit T0 = B0, T1 = B1, T2 = B2, T3 = B3, T4;
    T4  = T2; T2 ^= T0; T0 &= T3; T4 |= T3; T2 = ~T2; T3 ^= T1; T1 |= T0;
    T0 ^= T2; T2 &= T4; T3 &= T4; T1 ^= T2; T2 ^= T0; T0 |= T2; T4 ^= T1;
    T0 ^= T3; T3 ^= T4; T4 |= T0; T3 ^= T2; T4 ^= T2;
    B0 = T3; B1 = T0; B2 = T1; B3 = T4;
}

/*************************************************
* Serpent's Linear Transformation                *
*************************************************/
inline void transform(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    B0  = rotate_left(B0, 13);   B2  = rotate_left(B2, 3);
    B1 ^= B0 ^ B2;               B3 ^= B2 ^ (B0 << 3);
    B1  = rotate_left(B1, 1);    B3  = rotate_left(B3, 7);
    B0 ^= B1 ^ B3;               B2 ^= B3 ^ (B1 << 7);
    B0  = rotate_left(B0, 5);    B2  = rotate_left(B2, 22);
}

/*************************************************
* Serpent's Inverse Linear Transformation        *
*************************************************/
inline void i_transform(u32bit& B0, u32bit& B1, u32bit& B2, u32bit& B3)
{
    B2  = rotate_right(B2, 22);  B0  = rotate_right(B0, 5);
    B2 ^= B3 ^ (B1 << 7);        B0 ^= B1 ^ B3;
    B3  = rotate_right(B3, 7);   B1  = rotate_right(B1, 1);
    B3 ^= B2 ^ (B0 << 3);        B1 ^= B0 ^ B2;
    B2  = rotate_right(B2, 3);   B0  = rotate_right(B0, 13);
}

/*************************************************
* XOR a key block with a data block              *
*************************************************/
#define key_xor(round, B0, B1, B2, B3) \
    B0 ^= round_key[4*round  ];	       \
    B1 ^= round_key[4*round+1];	       \
    B2 ^= round_key[4*round+2];	       \
    B3 ^= round_key[4*round+3];

/*************************************************
* Serpent Encryption                             *
*************************************************/

void SerpentECB::encrypt_block(const uint8_t src[16], uint8_t dst[16]) const
{
    u32bit B0 = load_le<u32bit>(src, 0);
    u32bit B1 = load_le<u32bit>(src, 1);
    u32bit B2 = load_le<u32bit>(src, 2);
    u32bit B3 = load_le<u32bit>(src, 3);

    key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

    store_le(dst, B0, B1, B2, B3);
}

void SerpentECB::encrypt(const void* src, void* dst, size_t len) const
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	encrypt_block((uint8_t*)src + i, (uint8_t*)dst + i);
    }
}

void SerpentECB::encrypt(void* srcdst, size_t len) const
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	encrypt_block((uint8_t*)srcdst + i, (uint8_t*)srcdst + i);
    }
}

void SerpentCBC::encrypt_block(const uint8_t src[16], uint8_t dst[16])
{
    u32bit B0 = load_le<u32bit>(src, 0) ^ l_cbciv[0];
    u32bit B1 = load_le<u32bit>(src, 1) ^ l_cbciv[1];
    u32bit B2 = load_le<u32bit>(src, 2) ^ l_cbciv[2];
    u32bit B3 = load_le<u32bit>(src, 3) ^ l_cbciv[3];

    key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
    key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

    store_le(dst, B0, B1, B2, B3);

    l_cbciv[0] = B0;
    l_cbciv[1] = B1;
    l_cbciv[2] = B2;
    l_cbciv[3] = B3;
}

void SerpentCBC::encrypt(const void* src, void* dst, size_t len)
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	encrypt_block((uint8_t*)src + i, (uint8_t*)dst + i);
    }
}

void SerpentCBC::encrypt(void* srcdst, size_t len)
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	encrypt_block((uint8_t*)srcdst + i, (uint8_t*)srcdst + i);
    }
}

/*************************************************
* Serpent Decryption                             *
*************************************************/

void SerpentECB::decrypt_block(const uint8_t src[16], uint8_t dst[16]) const
{
    u32bit B0 = load_le<u32bit>(src, 0);
    u32bit B1 = load_le<u32bit>(src, 1);
    u32bit B2 = load_le<u32bit>(src, 2);
    u32bit B3 = load_le<u32bit>(src, 3);

    key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

    store_le(dst, B0, B1, B2, B3);
}

void SerpentECB::decrypt(const void* src, void* dst, size_t len) const
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	decrypt_block((uint8_t*)src + i, (uint8_t*)dst + i);
    }
}

void SerpentECB::decrypt(void* srcdst, size_t len) const
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	decrypt_block((uint8_t*)srcdst + i, (uint8_t*)srcdst + i);
    }
}

void SerpentCBC::decrypt_block(const uint8_t src[16], uint8_t dst[16])
{
    u32bit B0 = load_le<u32bit>(src, 0);
    u32bit B1 = load_le<u32bit>(src, 1);
    u32bit B2 = load_le<u32bit>(src, 2);
    u32bit B3 = load_le<u32bit>(src, 3);

    u32bit A0 = B0, A1 = B1, A2 = B2, A3 = B3;

    key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
    i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

    store_le(dst,
	     B0 ^ l_cbciv[0],
	     B1 ^ l_cbciv[1],
	     B2 ^ l_cbciv[2],
	     B3 ^ l_cbciv[3]);

    l_cbciv[0] = A0;
    l_cbciv[1] = A1;
    l_cbciv[2] = A2;
    l_cbciv[3] = A3;
}

void SerpentCBC::decrypt(const void* src, void* dst, size_t len)
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	decrypt_block((uint8_t*)src + i, (uint8_t*)dst + i);
    }
}

void SerpentCBC::decrypt(void* srcdst, size_t len)
{
    assert(len % 16 == 0);

    for (unsigned int i = 0; i < len; i += 16)
    {
	decrypt_block((uint8_t*)srcdst + i, (uint8_t*)srcdst + i);
    }
}

/*************************************************
* Serpent Key Schedule                           *
*************************************************/

void SerpentECB::set_key(const unsigned char* key, unsigned int keylen)
{
    const u32bit PHI = 0x9E3779B9;

    u32bit W[140];
    for(u32bit j = 0; j != keylen / 4; ++j)
	W[j] = load_le<u32bit>(key, j);

    W[keylen / 4] |= u32bit(1) << ((keylen%4)*8);
    for(u32bit j = 8; j != 140; ++j)
	W[j] = rotate_left(W[j-8] ^ W[j-5] ^ W[j-3] ^ W[j-1] ^ PHI ^ (j-8), 11);

    SBoxE4(W[  8],W[  9],W[ 10],W[ 11]); SBoxE3(W[ 12],W[ 13],W[ 14],W[ 15]);
    SBoxE2(W[ 16],W[ 17],W[ 18],W[ 19]); SBoxE1(W[ 20],W[ 21],W[ 22],W[ 23]);
    SBoxE8(W[ 24],W[ 25],W[ 26],W[ 27]); SBoxE7(W[ 28],W[ 29],W[ 30],W[ 31]);
    SBoxE6(W[ 32],W[ 33],W[ 34],W[ 35]); SBoxE5(W[ 36],W[ 37],W[ 38],W[ 39]);
    SBoxE4(W[ 40],W[ 41],W[ 42],W[ 43]); SBoxE3(W[ 44],W[ 45],W[ 46],W[ 47]);
    SBoxE2(W[ 48],W[ 49],W[ 50],W[ 51]); SBoxE1(W[ 52],W[ 53],W[ 54],W[ 55]);
    SBoxE8(W[ 56],W[ 57],W[ 58],W[ 59]); SBoxE7(W[ 60],W[ 61],W[ 62],W[ 63]);
    SBoxE6(W[ 64],W[ 65],W[ 66],W[ 67]); SBoxE5(W[ 68],W[ 69],W[ 70],W[ 71]);
    SBoxE4(W[ 72],W[ 73],W[ 74],W[ 75]); SBoxE3(W[ 76],W[ 77],W[ 78],W[ 79]);
    SBoxE2(W[ 80],W[ 81],W[ 82],W[ 83]); SBoxE1(W[ 84],W[ 85],W[ 86],W[ 87]);
    SBoxE8(W[ 88],W[ 89],W[ 90],W[ 91]); SBoxE7(W[ 92],W[ 93],W[ 94],W[ 95]);
    SBoxE6(W[ 96],W[ 97],W[ 98],W[ 99]); SBoxE5(W[100],W[101],W[102],W[103]);
    SBoxE4(W[104],W[105],W[106],W[107]); SBoxE3(W[108],W[109],W[110],W[111]);
    SBoxE2(W[112],W[113],W[114],W[115]); SBoxE1(W[116],W[117],W[118],W[119]);
    SBoxE8(W[120],W[121],W[122],W[123]); SBoxE7(W[124],W[125],W[126],W[127]);
    SBoxE6(W[128],W[129],W[130],W[131]); SBoxE5(W[132],W[133],W[134],W[135]);
    SBoxE4(W[136],W[137],W[138],W[139]);

    memcpy(round_key, W + 8, 132 * sizeof(u32bit));
}

void SerpentCBC::set_key(const unsigned char* key, unsigned int keylen)
{
    const u32bit PHI = 0x9E3779B9;

    u32bit W[140];
    for(u32bit j = 0; j != keylen / 4; ++j)
	W[j] = load_le<u32bit>(key, j);

    W[keylen / 4] |= u32bit(1) << ((keylen%4)*8);
    for(u32bit j = 8; j != 140; ++j)
	W[j] = rotate_left(W[j-8] ^ W[j-5] ^ W[j-3] ^ W[j-1] ^ PHI ^ (j-8), 11);

    SBoxE4(W[  8],W[  9],W[ 10],W[ 11]); SBoxE3(W[ 12],W[ 13],W[ 14],W[ 15]);
    SBoxE2(W[ 16],W[ 17],W[ 18],W[ 19]); SBoxE1(W[ 20],W[ 21],W[ 22],W[ 23]);
    SBoxE8(W[ 24],W[ 25],W[ 26],W[ 27]); SBoxE7(W[ 28],W[ 29],W[ 30],W[ 31]);
    SBoxE6(W[ 32],W[ 33],W[ 34],W[ 35]); SBoxE5(W[ 36],W[ 37],W[ 38],W[ 39]);
    SBoxE4(W[ 40],W[ 41],W[ 42],W[ 43]); SBoxE3(W[ 44],W[ 45],W[ 46],W[ 47]);
    SBoxE2(W[ 48],W[ 49],W[ 50],W[ 51]); SBoxE1(W[ 52],W[ 53],W[ 54],W[ 55]);
    SBoxE8(W[ 56],W[ 57],W[ 58],W[ 59]); SBoxE7(W[ 60],W[ 61],W[ 62],W[ 63]);
    SBoxE6(W[ 64],W[ 65],W[ 66],W[ 67]); SBoxE5(W[ 68],W[ 69],W[ 70],W[ 71]);
    SBoxE4(W[ 72],W[ 73],W[ 74],W[ 75]); SBoxE3(W[ 76],W[ 77],W[ 78],W[ 79]);
    SBoxE2(W[ 80],W[ 81],W[ 82],W[ 83]); SBoxE1(W[ 84],W[ 85],W[ 86],W[ 87]);
    SBoxE8(W[ 88],W[ 89],W[ 90],W[ 91]); SBoxE7(W[ 92],W[ 93],W[ 94],W[ 95]);
    SBoxE6(W[ 96],W[ 97],W[ 98],W[ 99]); SBoxE5(W[100],W[101],W[102],W[103]);
    SBoxE4(W[104],W[105],W[106],W[107]); SBoxE3(W[108],W[109],W[110],W[111]);
    SBoxE2(W[112],W[113],W[114],W[115]); SBoxE1(W[116],W[117],W[118],W[119]);
    SBoxE8(W[120],W[121],W[122],W[123]); SBoxE7(W[124],W[125],W[126],W[127]);
    SBoxE6(W[128],W[129],W[130],W[131]); SBoxE5(W[132],W[133],W[134],W[135]);
    SBoxE4(W[136],W[137],W[138],W[139]);

    memcpy(round_key, W + 8, 132 * sizeof(u32bit));
}

void SerpentCBC::set_cbciv(const uint8_t iv[16])
{
    l_cbciv[0] = ((uint32_t*)iv)[0];
    l_cbciv[1] = ((uint32_t*)iv)[1];
    l_cbciv[2] = ((uint32_t*)iv)[2];
    l_cbciv[3] = ((uint32_t*)iv)[3];
}

/*** end of serpent.c ***/

void SerpentECB::wipe()
{
    memset(round_key, 0xFF, sizeof(round_key));
    memset(round_key, 0x00, sizeof(round_key));
}

void SerpentCBC::wipe()
{
    memset(round_key, 0xFF, sizeof(round_key));
    memset(round_key, 0x00, sizeof(round_key));
    memset(round_key, 0xFF, sizeof(l_cbciv));
    memset(round_key, 0x00, sizeof(l_cbciv));
}

} // namespace internal
} // namespace Enctain
