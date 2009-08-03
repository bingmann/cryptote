// $Id$

/*
 * CryptoTE v0.0.0
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <wx/mstream.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>

#include <time.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <zlib.h>

// *** Missing in 2.8, but always added by wxGlade ***
#ifndef wxTHICK_FRAME
#define wxTHICK_FRAME wxRESIZE_BORDER
#endif

// *** Some functions to load a compiled-in transparent PNG as wxBitmap or
// *** wxIcon

#define wxBitmapFromMemory(name) wxBitmapFromMemory2(name, sizeof(name))

static inline wxBitmap wxBitmapFromMemory2(const char *data, int len) {
    wxMemoryInputStream is(data, len);
    return wxBitmap(wxImage(is, wxBITMAP_TYPE_PNG, -1), -1);
}

#define wxIconFromMemory(name) wxIconFromMemory2(name, sizeof(name))

static inline wxIcon wxIconFromMemory2(const char *data, int len) {
    wxIcon icon;
    icon.CopyFromBitmap( wxBitmapFromMemory2(data, len) );
    return icon;
}

// *** Somewhat safe conversions between wxString and std::string ***

static inline wxString strSTL2WX(const std::string& str) {
    return wxString(str.data(), wxConvUTF8, str.size());
}

static inline std::string strWX2STL(const wxString& str) {
#if wxUSE_UNICODE
    size_t outlen;
    const wxCharBuffer cbuf = wxConvUTF8.cWC2MB(str.GetData(), str.Length(), &outlen);
    return std::string(cbuf.data(), outlen);
#else
    return std::string(str.GetData(), str.Length());
#endif
}

// *** Return a string representation of T ***

template <typename T>
static inline std::string toSTLString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// *** Return the current unix timestamp packed into a std::string ***

static inline std::string strTimeStampNow() {
    time_t timenow = time(NULL);
    return std::string((char*)&timenow, sizeof(timenow));
}

// *** Decompression with zlib ***

/**
 * Decompress a string using zlib and return the original data. Throws
 * std::runtime_error if an error occurred during decompression.
 */
static inline std::string decompress(const char* str, unsigned int slen, unsigned int possiblelen=0)
{
    z_stream zs;	// z_stream is zlib's control structure
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
	throw(std::runtime_error("inflateInit failed while decompressing."));

    zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(str));
    zs.avail_in = slen;

    int ret;
    char outbuffer[32768];
    std::string outstring;
    outstring.reserve(possiblelen);

    // get the uncompressed bytes blockwise using repeated calls to inflate
    do {
	zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
	zs.avail_out = sizeof(outbuffer);

	ret = inflate(&zs, 0);

	if (outstring.size() < zs.total_out) {
	    outstring.append(outbuffer,
			     zs.total_out - outstring.size());
	}

    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
	std::ostringstream oss;
	oss << "Exception during zlib uncompression: (" << ret << ") "
	    << zs.msg;
	throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

#endif // __TOOLS_H__
