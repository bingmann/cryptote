/*******************************************************************************
 * libenctain/testsuite/test_sha256.cpp
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

#include <assert.h>
#include <iostream>

#include "botan-1.6/include/init.h"
#include "botan-1.6/include/sha256.h"
#include "botan-1.6/include/hmac.h"
#include "botan-1.6/include/pkcs5.h"

using namespace Enctain;

int main()
{
    Botan::LibraryInitializer init;

    std::cout << "Testing SHA256 related functions...";
    std::cout.flush();

    {
        std::string testdata("lWqojf6XamHoE0fTdVGi5jl0cWaDAgH930EJDA5TNcZJVhG375", 50);
        Botan::OctetString out = Botan::SHA_256().process(testdata);

        Botan::OctetString cmp("D46BC93C4CE6E34C46483A0D1B6E51CB507BE33E19BB981D990D291EA6D79F62");
        assert(out == cmp);
    }

    {
        Botan::HMAC hmac("SHA-256");

        Botan::OctetString testkey((Botan::byte*)"MO8ZXS1JMiPRj9Sx0VUFtlNJUNNX3V54kgNyDlLn3pxzCFxYZE", 50);
        Botan::OctetString testdata((Botan::byte*)"CdDPEafk40BDHshs50EJvjhzbCMf9jDEihKWQ2HAEXfxwCaGqM", 50);

        hmac.set_key(testkey.bits_of());
        Botan::OctetString out = hmac.process(testdata.bits_of());

        Botan::OctetString cmp("E268B3BD6191FF857E22662F7C408C90485F5430A25D899CAE175105D50AFEA6");
        assert(out == cmp);
    }

    {
        Botan::PKCS5_PBKDF2 pbkdf("SHA-256");

        Botan::OctetString testsalt((Botan::byte*)"HnDN1IKwKyUwgJjTwYFHtjM1AdpS1y4yPxYDMtFzAU9a54PYIn", 50);

        pbkdf.set_iterations(4128);
        pbkdf.change_salt(testsalt.bits_of());

        Botan::OctetString testpassword((Botan::byte*)"eIU5cm9dVCJOgITU7svqcGlgLj1kTk3SpsU2LtHN5dZ3yc2tc2", 50);
        Botan::OctetString out = pbkdf.derive_key(64, testpassword.bits_of());

        Botan::OctetString cmp("216BD1F833F9978C0DF30FE737E77B48EE4419AE4655184734864EC09D55385B"
                               "60AE14320BA200DC654970D6AF7BA1BD4A6545D71CEA9000064861C9245D44CD");
        assert(out == cmp);
    }

    std::cout << "OK\n";

    return 0;
}

/******************************************************************************/
