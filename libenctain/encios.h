// $Id$

#ifndef ENCTAIN_ENCIOS_H
#define ENCTAIN_ENCIOS_H

#include <ostream>
#include <istream>
#include "enctain.h"

namespace Enctain {

/**
 * Implementation of the abstract interface DataOutput. Writes data out into
 * the given std::ostream object.
 */
struct DataOutputStream : public DataOutput
{
private:
    /// Output stream used.
    std::ostream&	os;

public:
    /// Constructor is passed the output stream.
    DataOutputStream(std::ostream& s)
	: os(s)
    {
    }

    /// Virtual implementation writes out the data.
    virtual bool Output(const void* data, size_t datalen)
    {
	return os.write((const char*)data, datalen).good();
    }
};

/**
 * Implementation of the abstract interface DataInput. Attempts to read data
 * from the given std::istream object.
 */
struct DataInputStream : public DataInput
{
private:
    /// Input stream used.
    std::istream&	is;

public:
    /// Constructor is passed the input stream.
    DataInputStream(std::istream& s)
	: is(s)
    {
    }

    /// Virtual implementation attempts to read maxlen data.
    virtual unsigned int Input(void* data, size_t maxlen)
    {
	return is.read((char*)data, maxlen).gcount();
    }
};

} // namespace Enctain

#endif // ENCTAIN_ENCIOS_H
