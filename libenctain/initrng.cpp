// $Id$

/*
 * CryptoTE LibEnctain v0.5.377
 * Copyright (C) 2008-2009 Timo Bingmann
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "enctain.h"

#include "botan-1.6/include/init.h"
#include "botan-1.6/include/rng.h"

namespace Enctain {

// *** Library Initialization and Shutdown Object ***

void LibraryInitializer::initialize(const std::string& args)
{
    Botan::LibraryInitializer::initialize(args);
}

void LibraryInitializer::deinitialize()
{
    Botan::LibraryInitializer::deinitialize();
}

// *** Random Number Generator Forwarders ***

void RNG::randomize(unsigned char* out, unsigned int len)
{
    Botan::Global_RNG::randomize(out, len);
}

unsigned char RNG::random_byte()
{
    return Botan::Global_RNG::random();
}

unsigned int RNG::random_uint()
{
    unsigned int val;
    randomize((unsigned char*)&val, sizeof(val));
    return val;
}

} // namespace Enctain
