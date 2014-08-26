/*******************************************************************************
 * libenctain/testsuite/test_ect2.cpp
 *
 * Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote
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

#include "enctain.h"
#include "encios.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <iostream>

#include "botan-1.6/include/crc32.h"

static inline uint32_t botan_crc32(const void* data, uint32_t datalen)
{
    using namespace Enctain;
    Botan::OctetString crc = Botan::CRC32().process((const Botan::byte*)data, datalen);
    assert(crc.length() == 4);
    return *(uint32_t*)crc.begin();
}

void test_frozen1_ect()
{
    Enctain::Container container;

    std::string filepath;
    if (getenv("srcdir")) {
        // this is required if the package is built outside it's srcdir
        filepath = getenv("srcdir") + std::string("/");
    }
    filepath += "frozen1.ect";

    std::ifstream instream(filepath.c_str());
    Enctain::DataInputStream datain(instream);

    container.Load(datain, "Pbm3jVnllHvB8YIIEhUPXjc13UOn0nqK");

    // Test Global Data

    assert(container.GetGlobalUnencryptedProperty("Author") == "tb");
    assert(container.GetGlobalUnencryptedProperty("Subject") == "File for test case");

    assert(container.GetGlobalEncryptedProperty("DefaultEncryption") == "1");
    assert(container.GetGlobalEncryptedProperty("DefaultCompression") == "1");

    assert(container.CountSubFile() == 3);

#if 0
    for (unsigned int pi = 0; ; ++pi)
    {
        std::string key, val;
        if (!container.GetGlobalUnencryptedPropertyIndex(pi, key, val)) break;

        std::cout << key << " => " << val << "\n";
    }
#endif
#if 0
    for (unsigned int pi = 0; ; ++pi)
    {
        std::string key, val;
        if (!container.GetGlobalEncryptedPropertyIndex(pi, key, val)) break;

        std::cout << key << " => " << val << "\n";
    }
#endif
#if 0
    for (unsigned int pi = 0; ; ++pi)
    {
        std::string key, val;
        if (!container.GetSubFilePropertyIndex(2, pi, key, val)) break;

        std::cout << key << " => " << val << "\n";
    }
#endif

    // Test SubFile 0

    assert(container.GetSubFileProperty(0, "Name") == "Das Lied von der Glocke.txt");
    assert(container.GetSubFileProperty(0, "Description") == "from Project Gutenberg");
    assert(container.GetSubFileProperty(0, "Subject") == "Friedrich von Schiller - Das Lied von der Glocke");

    assert(container.GetSubFileCompression(0) == Enctain::COMPRESSION_NONE);
    assert(container.GetSubFileEncryption(0) == Enctain::ENCRYPTION_SERPENT256);

    {
        std::string subfiledata;
        container.GetSubFileData(0, subfiledata);

        uint32_t crc32 = botan_crc32(subfiledata.data(), subfiledata.size());
        assert(crc32 == 0xC625504D);
    }

    // Test SubFile 1

    assert(container.GetSubFileProperty(1, "Name") == "Alice in Wonderland.txt");
    assert(container.GetSubFileProperty(1, "Description") == "from Project Gutenberg");
    assert(container.GetSubFileProperty(1, "Subject") == "Lewis Carroll - Alice in Wonderland");

    assert(container.GetSubFileCompression(1) == Enctain::COMPRESSION_ZLIB);
    assert(container.GetSubFileEncryption(1) == Enctain::ENCRYPTION_SERPENT256);

    {
        std::string subfiledata;
        container.GetSubFileData(1, subfiledata);

        uint32_t crc32 = botan_crc32(subfiledata.data(), subfiledata.size());
        assert(crc32 == 0x8785B082);
    }

    // Test SubFile 2

    assert(container.GetSubFileProperty(2, "Name") == "Macbeth.txt");
    assert(container.GetSubFileProperty(2, "Description") == "from Project Gutenberg");
    assert(container.GetSubFileProperty(2, "Subject") == "William Shakespeare - Macbeth");

    assert(container.GetSubFileCompression(2) == Enctain::COMPRESSION_BZIP2);
    assert(container.GetSubFileEncryption(2) == Enctain::ENCRYPTION_SERPENT256);

    {
        std::string subfiledata;
        container.GetSubFileData(2, subfiledata);

        uint32_t crc32 = botan_crc32(subfiledata.data(), subfiledata.size());
        assert(crc32 == 0x6964D83F);
    }
}

int main()
{
    Enctain::LibraryInitializer init;

    srand(time(NULL));

    std::cout << "Reading Frozen Encrypted Container...";
    std::cout.flush();

    Enctain::Container::SetSignature("CryptoTE");

    test_frozen1_ect();

    std::cout << "OK\n";

    return 0;
}

/******************************************************************************/
