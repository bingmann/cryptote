// $Id$

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

namespace Enctain {

/// Return the CRC32 of the bytes buf[0..len-1].
uint32_t crc32(const unsigned char *buf, unsigned int len);

/// Update a running crc32 with the bytes buf[0..len-1] and return the updated
/// crc32.
uint32_t update_crc32(uint32_t crc, const unsigned char *buf, unsigned int len);

} // namespace Enctain

#endif // CRC32_H
