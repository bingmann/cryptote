// $Id$

#ifndef __PRNG_H__
#define __PRNG_H__

#include <stdlib.h>

/**
 * PRNG implements a pseudo-random number generator based on the system random
 * number generator. The code is partially based on libtomcrypt. It fetches
 * random numbers from /dev/urandom or from the Win32
 * CryptographicServiceProvider.
 */

class PRNG
{
private:
    /// Buffer for random bytes.
    unsigned char	data_[64];

    /// Current length of the buffer
    unsigned int	len_;

    /// Current used bytes of the buffer.
    unsigned int	used_;

public:
    /// Initialize random generator
    PRNG();

    /// Return a 8-bit random byte.
    unsigned char	get_byte();

    /// Return a 32-bit random integer.
    unsigned int	get_int32();

    /// Return a random integer from [0..limit) excluding.
    unsigned int	get(unsigned int limit);

protected:
    
    /// Refill the random number buffer.
    void		generate();

    /// Read /dev/urandom for new random bytes
    size_t		generate_unix();

    /// Read the Windows CSP for new random bytes.
    size_t		generate_win32();

    /// Perform idle clock timing to get new random bytes.
    size_t		generate_clock();
};

#endif // __PRNG_H__
