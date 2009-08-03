// $Id$

/*
 * CryptoTE LibEnctain v0.0.0
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

#ifndef BYTEBUFF_H
#define BYTEBUFF_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <utility>
#include <string>
#include <stdexcept>

namespace Enctain {
namespace internal {

/**
 * ByteBuffer is a byte-oriented memory buffer which is very useful to flexibly
 * build and parse variable-sized data structures. It holds an automatically
 * growing byte buffer. This buffer can be either filled with append() and
 * put() or parsed using get() which are template functions appending or
 * retreiving any integral data type. It has two template specializations to
 * save/load std::string by prefixing it's length.
 */

class ByteBuffer
{
private:
    /// Allocated buffer pointer.
    unsigned char*	m_data;

    /// Size of valid data.
    unsigned int	m_size;

    /// Total size of buffer.
    unsigned int	m_buff;

    /// Current read cursor.
    unsigned int	m_curr;

public:
    /// Create a new empty object
    inline ByteBuffer() {
	m_data = NULL;
	m_size = m_buff = m_curr = 0;
    }

    /// Create an object with n bytes preallocated
    inline ByteBuffer(unsigned int n) {
	m_data = NULL;
	m_size = m_buff = m_curr = 0;
	alloc(n);
    }

    /// Copy-Constructor, duplicate memory pointer.
    inline ByteBuffer(const ByteBuffer& other) {
	m_data = NULL;
	m_size = m_buff = m_curr = 0;
	assign(other);
    }

    /// Destroys the memory space.
    ~ByteBuffer() {
	dealloc();
    }

    /// Return a pointer to the currently kept memory area.
    inline const unsigned char *data() const
    { return m_data; }

    /// Return a writeable pointer to the currently kept memory area.
    inline unsigned char *data()
    { return m_data; }

    /// Return the currently used length in bytes.
    inline unsigned int size() const
    { return m_size; }

    /// Return the currently allocated buffer size.
    inline unsigned int buffsize() const
    { return m_buff; }

    /// Set the valid bytes in the buffer, used if the buffer is filled
    /// directly.
    inline void set_size(unsigned int n)
    { assert(n <= m_buff); m_size = n; }

    /// Explicit conversion to std::string (copies memory of course).
    inline std::string str() const
    { return std::string(reinterpret_cast<const char*>(m_data), m_size); }

    /// Make sure that at least n bytes are allocated.
    inline void alloc(unsigned int n)
    {
	if (m_buff < n)
	{
	    m_buff = n;
	    m_data = static_cast<unsigned char*>(realloc(m_data, m_buff));
	}
    }

    /// Dynamically allocate more memory. At least n bytes will be available,
    /// probably more to compensate future growth.
    inline void dynalloc(unsigned int n)
    {
	if (m_buff < n)
	{
	    // place to adapt the buffer growing algorithm as need.
	    unsigned int newsize = m_buff;

	    while(newsize < n) {
		if (newsize < 256) newsize = 512;
		else if (newsize < 1024*1024) newsize = 2*newsize;
		else newsize += 1024*1024;
	    }

	    alloc(newsize);
	}
    }

    /// Clears the memory contents, does not deallocate the memory.
    inline void clear()
    { m_size = 0; }

    /// Deallocates the kept memory space (we use dealloc() instead of free()
    /// as a name, because sometimes "free" is replaced by the preprocessor)
    inline void dealloc()
    {
	if (m_data) free(m_data);
	m_data = NULL;
	m_size = m_buff = m_curr = 0;
    }

    /// Detach the memory from the object, returns the memory pointer.
    inline const unsigned char* detach()
    {
	const unsigned char* data = m_data;
	m_data = NULL;
	m_size = m_buff = m_curr = 0;
	return data;
    }

    /// Copy a memory range into the buffer, overwrites all current
    /// data. Roughly equivalent to clear() followed by append().
    inline void assign(const void *indata, unsigned int inlen)
    {
	if (inlen > m_buff) alloc(inlen);

	memcpy(m_data, indata, inlen);
	m_size = inlen;
    }

    /// Copy the contents of another buffer object into this buffer, overwrites
    /// all current data. Roughly equivalent to clear() followed by append().
    inline void assign(const ByteBuffer& other)
    {
	if (&other == this) return;
	assign(other.data(), other.size());
    }

    /// Assignment operator: copy other's memory range into buffer.
    inline ByteBuffer& operator=(const ByteBuffer& other)
    {
	if (&other == this) return *this;

	assign(other.data(), other.size());
	return *this;
    }

    // *** Appending Write Functions ***

    /// Append a memory range to the buffer
    inline void append(const void *indata, unsigned int inlen)
    {
	if (m_size + inlen > m_buff) dynalloc(m_size + inlen);

	memcpy(m_data + m_size, indata, inlen);
	m_size += inlen;
    }

    /// Append the contents of a different buffer object to this one.
    inline void append(const class ByteBuffer &bb)
    { append(bb.data(), bb.size()); }

    /// Append to contents of a std::string, excluding the null (which isn't
    /// contained in the string size anyway).
    inline void append(const std::string &s)
    { append(s.data(), s.size()); }

    /// Put (append) a single item of the template type T to the buffer. Be
    /// careful with implicit type conversions!
    template <typename Tp>
    inline void put(const Tp item)
    {
	if (m_size + sizeof(Tp) > m_buff) dynalloc(m_size + sizeof(Tp));

	*reinterpret_cast<Tp*>(m_data + m_size) = item;
	m_size += sizeof(Tp);
    }

    /// Align the size of the buffer to a multiple of n. Fills up with 0s.
    void align(unsigned int n)
    {
	assert(n > 0);
	unsigned int rem = m_size % n;
	if (rem != 0)
	{
	    unsigned int add = n - rem;
	    if (m_size + add > m_buff) dynalloc(m_size + add);
	    memset(m_data + m_size, 0, add);
	    m_size += add;
	}
	assert((m_size % n) == 0);
    }

    // *** Cursor-Driven Read Functions ***

    /// Reset the read cursor.
    inline void rewind()
    { m_curr = 0; }

    /// Check that n bytes are available at the cursor.
    inline bool cursor_available(unsigned int n) const
    { return (m_curr + n <= m_size); }

    /// Throws a std::underflow_error unless n bytes are available at the
    /// cursor.
    inline void check_available(unsigned int n) const {
	if (!cursor_available(n))
	    throw(std::underflow_error("ByteBuffer underrun"));
    }

    /// Fetch a single item of the template type Tp from the buffer, advancing
    /// the cursor. Be careful with implicit type conversions!
    template <typename Tp>
    inline Tp get()
    {
	check_available(sizeof(Tp));

	Tp ret = *reinterpret_cast<Tp*>(m_data + m_curr);
	m_curr += sizeof(Tp);

	return ret;
    }

    /// Fetch a number of unstructured bytes from the buffer, advancing the cursor
    inline void get(void* outdata, unsigned int datalen)
    {
	check_available(datalen);
	memcpy(outdata, m_data + m_curr, datalen);
	m_curr += datalen;
    }
};

/// A template specialization which appends a std::string by first putting it's
/// length as one unsigned byte (or escaped to unsigned four bytes integer),
/// followed by it's data.
template <>
inline void ByteBuffer::put<std::string>(const std::string item)
{
    if (item.size() < 0xFF) {
	put<unsigned char>(item.size());
    }
    else {
	put<unsigned char>(0xFF);
	put<unsigned int>(item.size());
    }

    append(item);
}

/// The corresponding template specialization which extracts the std::string.
template <>
inline std::string ByteBuffer::get<std::string>()
{
    unsigned int slen = get<unsigned char>();
    if (slen == 0xFF) {
	slen = get<unsigned int>();
    }

    check_available(slen);
    std::string ret(reinterpret_cast<const char*>(m_data + m_curr), slen);

    m_curr += slen;
    return ret;
}

} // namespace internal
} // namespace Enctain

#endif // BYTEBUFF_H
