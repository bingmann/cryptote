// $Id$

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
