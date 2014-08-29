/*******************************************************************************
 * libenctain/testsuite/test_serpent.cpp
 *
 * Part of CryptoTE v0.5.999, see http://panthema.net/2007/cryptote
 *******************************************************************************
 * Copyright (C) 2008-2014 Timo Bingmann <tb@panthema.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 ******************************************************************************/

#include <assert.h>
#include <iostream>

#include "botan-1.6/include/init.h"
#include "botan-1.6/include/pkcs5.h"
#include "botan-1.6/include/pipe.h"
#include "botan-1.6/include/lookup.h"
#include "botan-1.6/include/zlib.h"
#include "botan-1.6/include/bzip2.h"

using namespace Enctain;

int main()
{
    Botan::LibraryInitializer init;

    std::cout << "Testing Serpent related functions...";
    std::cout.flush();

    Botan::OctetString masterkey;

    {
        Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

        Botan::OctetString testsalt((Botan::byte*)"5BAtxShV61eLhiplqMPh7hI9tgpShnoi", 32);

        pbkdf.set_iterations(4128);
        pbkdf.change_salt(testsalt.bits_of());

        std::string testpassword = "bummer password";
        masterkey = pbkdf.derive_key(64, Botan::OctetString((Botan::byte*)testpassword.data(), testpassword.size()).bits_of());

        Botan::OctetString cmp("2609B2F18241E94113BE7724080391CCA7BB62D042393BDBD7DB72705512BAF8"
                               "24FFBD95623F1F2B480D7FE4987FA87D552313EA35C5136AACEAEFF2D676144A");
        assert(masterkey == cmp);
    }

    Botan::OctetString derived_key, derived_iv;

    {
        Botan::PKCS5_PBKDF2 pbkdf("SHA-256");
        pbkdf.set_iterations(4128);

        Botan::OctetString testsalt1((Botan::byte*)"CC3u7ZuhbfnAKVbCCkyvXJRaVnC1QhpL", 32);
        Botan::OctetString testsalt2((Botan::byte*)"wDsACfqAjOuwQQDyAlAA9N4mdvn5FnQf", 32);

        pbkdf.change_salt(testsalt1.bits_of());
        derived_key = pbkdf.derive_key(32, masterkey.bits_of());

        pbkdf.change_salt(testsalt2.bits_of());
        derived_iv = pbkdf.derive_key(16, masterkey.bits_of());

        Botan::OctetString cmp1("622392946C51C5490A2226DB102F46377B87DEBAE8CA3F07BB91E716293A6944");
        assert(derived_key == cmp1);

        Botan::OctetString cmp2("A7D32472113CB2069FA53FD2E241ADCE");
        assert(derived_iv == cmp2);
    }

    // Without Compression
    {
        const unsigned int bufferlen = 65530;

        Botan::SecureBuffer<Botan::byte, bufferlen> buffer_plaintext;

        for (unsigned int i = 0; i < bufferlen; ++i)
            buffer_plaintext[i] = (Botan::byte)(i * i);

        Botan::Pipe encpipe(
            new Botan::Fork(Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::ENCRYPTION),
                            new Botan::Hash_Filter("SHA-256"))
            );

        encpipe.start_msg();
        encpipe.write(buffer_plaintext);
        encpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_digest = encpipe.read_all(1);

        Botan::OctetString cmp1("000B9C31726C6629D0E999AF05015124DCA0929B0F6432DA2D5C3B02F271DF4C");
        assert(buffer_digest == cmp1);

        Botan::SecureVector<Botan::byte> buffer_ciphertext = encpipe.read_all(0);

        Botan::Pipe decpipe(
            Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::DECRYPTION)
            );

        decpipe.start_msg();
        decpipe.write(buffer_ciphertext);
        decpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_returntext = decpipe.read_all();

        assert(buffer_returntext.size() == bufferlen);
        for (unsigned int i = 0; i < bufferlen; ++i)
            assert(buffer_returntext[i] == (Botan::byte)(i * i));
    }

    // With zlib Compression
    {
        const unsigned int bufferlen = 65530;

        Botan::SecureBuffer<Botan::byte, bufferlen> buffer_plaintext;

        for (unsigned int i = 0; i < bufferlen; ++i)
            buffer_plaintext[i] = (Botan::byte)(i * i);

        Botan::Pipe encpipe(
            new Botan::Zlib_Compression(9),
            new Botan::Fork(Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::ENCRYPTION),
                            new Botan::Hash_Filter("SHA-256"))
            );

        encpipe.start_msg();
        encpipe.write(buffer_plaintext);
        encpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_digest = encpipe.read_all(1);

        Botan::OctetString cmp1("3E897F25F99E6597A3D81832A8753AC3A58A82C34B752322B0EEE92DEA0BEE19");
        assert(buffer_digest == cmp1);

        Botan::SecureVector<Botan::byte> buffer_ciphertext = encpipe.read_all(0);

        Botan::Pipe decpipe(
            Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::DECRYPTION),
            new Botan::Zlib_Decompression()
            );

        decpipe.start_msg();
        decpipe.write(buffer_ciphertext);
        decpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_returntext = decpipe.read_all();

        assert(buffer_returntext.size() == bufferlen);
        for (unsigned int i = 0; i < bufferlen; ++i)
            assert(buffer_returntext[i] == (Botan::byte)(i * i));
    }

    // With bzip2 Compression
    {
        const unsigned int bufferlen = 65530;

        Botan::SecureBuffer<Botan::byte, bufferlen> buffer_plaintext;

        for (unsigned int i = 0; i < bufferlen; ++i)
            buffer_plaintext[i] = (Botan::byte)(i * i);

        Botan::Pipe encpipe(
            new Botan::Bzip_Compression(9),
            new Botan::Fork(Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::ENCRYPTION),
                            new Botan::Hash_Filter("SHA-256"))
            );

        encpipe.start_msg();
        encpipe.write(buffer_plaintext);
        encpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_digest = encpipe.read_all(1);

        Botan::OctetString cmp1("C649411C3A6D5B284AB36BB21B0C7FF448B1C3A13682DB1DC34E7E2AF5253E8C");
        assert(buffer_digest == cmp1);

        Botan::SecureVector<Botan::byte> buffer_ciphertext = encpipe.read_all(0);

        Botan::Pipe decpipe(
            Botan::get_cipher("Serpent/CBC/PKCS7", derived_key, derived_iv, Botan::DECRYPTION),
            new Botan::Bzip_Decompression()
            );

        decpipe.start_msg();
        decpipe.write(buffer_ciphertext);
        decpipe.end_msg();

        Botan::SecureVector<Botan::byte> buffer_returntext = decpipe.read_all();

        assert(buffer_returntext.size() == bufferlen);
        for (unsigned int i = 0; i < bufferlen; ++i)
            assert(buffer_returntext[i] == (Botan::byte)(i * i));
    }

    std::cout << "OK\n";

    return 0;
}

/******************************************************************************/
