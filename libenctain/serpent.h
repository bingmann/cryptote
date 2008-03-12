// $Id$

#ifndef SERPENT_H
#define SERPENT_H

#include <stdint.h>
#include <stdlib.h>

namespace Enctain {

/**
 * Serpent encryption cipher state context to encrypt or decrypt data blocks in
 * ECB (Electronic codebook) mode. Directly based on the code from the Botan
 * cryptography library.
 */
class SerpentECB
{
private:
    /// storage for the key schedule
    uint32_t	round_key[132];

public:

    /// Set the encryption key. The key must be 16, 24 or 32 bytes long.
    void set_key(const unsigned char* key, unsigned int keylen);

    /// Encrypt a block of 16 bytes using the current cipher state.
    void encrypt_block(const uint8_t src[16], uint8_t dst[16]) const;

    /// Encrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void encrypt(const void* src, void* dst, size_t len) const;

    /// Encrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void encrypt(void* srcdst, size_t len) const;

    /// Decrypt a block of 16 bytes using the current cipher state.
    void decrypt_block(const uint8_t src[16], uint8_t dst[16]) const;

    /// Decrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void decrypt(const void* src, void* dst, size_t len) const;

    /// Decrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void decrypt(void* srcdst, size_t len) const;

    /// Wipe the current key schedule by overwriting the memory twice.
    void wipe();
};

/**
 * Serpent encryption cipher state context to encrypt or decrypt data blocks in
 * CBC (Cipher-block chaining) mode.
 */
class SerpentCBC
{
private:
    /// storage for the key schedule
    uint32_t	round_key[132];

    /// cbc initialisation vector
    uint32_t	l_cbciv[4];

public:

    /// Set the encryption key. The key must be 16, 24 or 32 bytes long.
    void set_key(const unsigned char* key, unsigned int keylen);

    /// Set the initial cbc vector. The vector is always 16 bytes long.
    void set_cbciv(const uint8_t iv[16]);

    /// Encrypt a block of 16 bytes using the current cipher state.
    void encrypt_block(const uint8_t src[16], uint8_t dst[16]);

    /// Encrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void encrypt(const void* src, void* dst, size_t len);

    /// Encrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void encrypt(void* srcdst, size_t len);

    /// Decrypt a block of 16 bytes using the current cipher state.
    void decrypt_block(const uint8_t src[16], uint8_t dst[16]);

    /// Decrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void decrypt(const void* src, void* dst, size_t len);

    /// Decrypt a length of n*16 bytes. Len must be a multiple of 16 or this
    /// function will assert().
    void decrypt(void* srcdst, size_t len);

    /// Wipe the current key schedule by overwriting the memory twice.
    void wipe();
};

} // namespace Enctain

#endif // SERPENT_H
