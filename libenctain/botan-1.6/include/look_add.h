/*************************************************
* Lookup Table Management Header File            *
* (C) 1999-2007 The Botan Project                *
*************************************************/

#ifndef BOTAN_LOOKUP_MANGEMENT_H__
#define BOTAN_LOOKUP_MANGEMENT_H__

#include "botan-1.6/include/base.h"
#include "botan-1.6/include/mode_pad.h"
#include "botan-1.6/include/s2k.h"

namespace Enctain {
namespace Botan {

/*************************************************
* Add an algorithm to the lookup table           *
*************************************************/
void add_algorithm(BlockCipher*);
void add_algorithm(StreamCipher*);
void add_algorithm(HashFunction*);
void add_algorithm(MessageAuthenticationCode*);
void add_algorithm(S2K*);
void add_algorithm(BlockCipherModePaddingMethod*);

}
}

#endif
