// $Id$

/*
 * CryptoTE LibEnctain v0.5.390
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
    std::ostream&	m_os;

public:
    /// Constructor is passed the output stream.
    DataOutputStream(std::ostream& os)
	: m_os(os)
    {
    }

    /// Virtual implementation writes out the data.
    virtual bool Output(const void* data, size_t datalen)
    {
	return m_os.write((const char*)data, datalen).good();
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
    std::istream&	m_is;

public:
    /// Constructor is passed the input stream.
    DataInputStream(std::istream& is)
	: m_is(is)
    {
    }

    /// Virtual implementation attempts to read maxlen data.
    virtual unsigned int Input(void* data, size_t maxlen)
    {
	return m_is.read((char*)data, maxlen).gcount();
    }
};

} // namespace Enctain

#endif // ENCTAIN_ENCIOS_H
