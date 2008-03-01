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

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <string>

namespace Enctain {

/**
 * SHA 256 bit state context to hash input data bytes.
 */

class SHA256
{
private:

    /// Keep total size for calculations
    unsigned int total;

    /// SHA state
    uint32_t	h[8];

    /// Buffer needed to group incoming bytes to 64 byte blocks
    uint8_t	buffer[64];

    /// SHA process a block of 64 bytes
    void	process(const uint8_t data[64]);

public:
    /// Initialize state structures
    SHA256();

    /// Reinitialize the state structures
    void	reset();

    /// Add a block of character data to the hash sum.
    void	update(const uint8_t* input, size_t len);

    /// Add a block of data to the hash sum. Just one type-cast less in your
    /// code.
    inline void update(const void* input, size_t len)
    { update(reinterpret_cast<const uint8_t*>(input), len); }

    /// Add a STL string to the hash sum.
    inline void update(const std::string& str)
    { update(str.data(), str.size()); }

    /// Finish hash calculation and return a pointer to the digest bytes. The
    /// returned buffer is 32 bytes long.
    const uint8_t* final();

    /// Finish hash calculation and return a pointer to the digest bytes. The
    /// returned buffer is 32 bytes long.
    std::string	final_str();

    /// Process the given string using SHA256 and return the digest in the
    /// string. The returned buffer is 32 bytes long.
    static std::string	digest(const std::string& data);
};

} // namespace Enctain

#endif // SHA256_H
