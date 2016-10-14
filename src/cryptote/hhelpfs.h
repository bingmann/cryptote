/*******************************************************************************
 * src/cryptote/hhelpfs.h
 *
 * Part of CryptoTE, see http://panthema.net/2007/cryptote
 *******************************************************************************
 * Copyright (C) 2008-2014 Timo Bingmann <tb@panthema.net>
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 ******************************************************************************/

#ifndef CRYPTOTE_SRC_HHELPFS_HEADER
#define CRYPTOTE_SRC_HHELPFS_HEADER

#include <string>
#include <wx/filesys.h>

/**
 * FileSystemHandler serving only the help: protocol with files from compressed
 * compiled-in data.
 */
class BuiltinHtmlHelpFSHandler : public wxFileSystemHandler
{
public:
    /// Does nothing.
    BuiltinHtmlHelpFSHandler();

    /// Required for virtual functions.
    ~BuiltinHtmlHelpFSHandler();

    /// Check that the location start with help: protocol.
    virtual bool CanOpen(const wxString& location);

    /// Tries to open the file specified.
    virtual wxFSFile * OpenFile(wxFileSystem& fs, const wxString& location);

protected:
    struct BuiltinFile
    {
        const wxString     path;
        const unsigned char* compressed_data;
        unsigned int       compressed_size;
        unsigned int       uncompressed_size;
        std::string        decompressed_data;
    };

    static struct BuiltinFile filelist[];
    static unsigned int filelistsize;

private:
    /// No copy construction allowed.
    BuiltinHtmlHelpFSHandler(const BuiltinHtmlHelpFSHandler&);

    /// No assignment allowed.
    BuiltinHtmlHelpFSHandler& operator = (const BuiltinHtmlHelpFSHandler&);
};

#endif // !CRYPTOTE_SRC_HHELPFS_HEADER

/******************************************************************************/
