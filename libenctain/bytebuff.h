// $Id$

#ifndef BYTEBUFF_H
#define BYTEBUFF_H

#include <stdlib.h>
#include <assert.h>

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
    char*	data_;

    /// Size of valid data.
    unsigned int size_;

    /// Total size of buffer.
    unsigned int buff_;

    /// Current read cursor.
    unsigned int curr_;

public:
    /// Create a new empty object
    inline ByteBuffer() {
	data_ = NULL;
	size_ = buff_ = curr_ = 0;
    }

    /// Create an object with n bytes preallocated
    inline ByteBuffer(unsigned int n) {
	data_ = NULL;
	size_ = buff_ = curr_ = 0;
	alloc(n);
    }

    /// Copy-Constructor, duplicate memory pointer.
    inline ByteBuffer(const ByteBuffer& other) {
	data_ = NULL;
	size_ = buff_ = curr_ = 0;
	assign(other);
    }

    /// Destroys the memory space.
    ~ByteBuffer() {
	dealloc();
    }

    /// Return a pointer to the currently kept memory area.
    inline const char *data() const
    { return data_; }

    /// Return a writeable pointer to the currently kept memory area.
    inline char *data()
    { return data_; }

    /// Return the currently used length in bytes.
    inline unsigned int size() const
    { return size_; }

    /// Return the currently allocated buffer size.
    inline unsigned int buffsize() const
    { return buff_; }

    /// Set the valid bytes in the buffer, used if the buffer is filled
    /// directly.
    inline void set_size(unsigned int n)
    { assert(n <= buff_); size_ = n; }

    /// Explicit conversion to std::string (copies memory of course).
    inline std::string str() const
    { return std::string(data_, size_); }

    /// Make sure that at least n bytes are allocated.
    inline void alloc(unsigned int n)
    {
	if (buff_ < n)
	{
	    buff_ = n;
	    data_ = static_cast<char*>(realloc(data_, buff_));
	}
    }

    /// Dynamically allocate more memory. At least n bytes will be available,
    /// probably more to compensate future growth.
    inline void dynalloc(unsigned int n)
    {
	if (buff_ < n)
	{
	    // place to adapt the buffer growing algorithm as need.
	    unsigned int newsize = buff_;

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
    { size_ = 0; }

    /// Deallocates the kept memory space (we use dealloc() instead of free()
    /// as a name, because sometimes "free" is replaced by the preprocessor)
    inline void dealloc()
    {
	if (data_) free(data_);
	data_ = NULL;
	size_ = buff_ = curr_ = 0;
    }

    /// Detach the memory from the object, returns the memory pointer.
    inline const char* detach()
    {
	const char* data = data_;
	data_ = NULL;
	size_ = buff_ = curr_ = 0;
	return data;
    }

    /// Copy a memory range into the buffer, overwrites all current
    /// data. Roughly equivalent to clear() followed by append().
    inline void assign(const void *indata, unsigned int inlen)
    {
	if (inlen > buff_) alloc(inlen);

	memcpy(data_, indata, inlen);
	size_ = inlen;
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
	if (size_ + inlen > buff_) dynalloc(size_ + inlen);

	memcpy(data_ + size_, indata, inlen);
	size_ += inlen;
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
	if (size_ + sizeof(Tp) > buff_) dynalloc(size_ + sizeof(Tp));

	*reinterpret_cast<Tp*>(data_ + size_) = item;
	size_ += sizeof(Tp);
    }

    /// Align the size of the buffer to a multiple of n. Fills up with 0s.
    void align(unsigned int n)
    {
	assert(n > 0);
	unsigned int rem = size_ % n;
	if (rem != 0)
	{
	    unsigned int add = n - rem;
	    if (size_ + add > buff_) dynalloc(size_ + add);
	    memset(data_ + size_, 0, add);
	    size_ += add;
	}
	assert((size_ % n) == 0);
    }

    // *** Cursor-Driven Read Functions ***

    /// Reset the read cursor.
    inline void rewind()
    { curr_ = 0; }

    /// Check that n bytes are available at the cursor.
    inline bool cursor_available(unsigned int n) const
    { return (curr_ + n <= size_); }

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

	Tp ret = *reinterpret_cast<Tp*>(data_ + curr_);
	curr_ += sizeof(Tp);

	return ret;
    }

    /// Fetch a number of unstructured bytes from the buffer, advancing the cursor
    inline void get(void* outdata, unsigned int datalen)
    {
	check_available(datalen);
	memcpy(outdata, data_ + curr_, datalen);
	curr_ += datalen;
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
    std::string ret(data_ + curr_, slen);

    curr_ += slen;
    return ret;
}

} // namespace internal
} // namespace Enctain

#endif // BYTEBUFF_H
