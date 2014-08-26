/*******************************************************************************
 * src/cryptote/hhelpfs.cpp
 *
 * Part of CryptoTE v0.0.0, see http://panthema.net/2007/cryptote
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

#include "hhelpfs.h"

#include "common/tools.h"
#include <wx/log.h>

BuiltinHtmlHelpFSHandler::BuiltinHtmlHelpFSHandler()
{ }

BuiltinHtmlHelpFSHandler::~BuiltinHtmlHelpFSHandler()
{ }

bool
BuiltinHtmlHelpFSHandler::CanOpen(const wxString& location)
{
    return (GetProtocol(location) == _T("help"));
}

wxFSFile*
BuiltinHtmlHelpFSHandler::OpenFile(wxFileSystem&, const wxString& location)
{
    if (location.Left(5) != _T("help:"))
        return NULL;

    wxString path = location.Mid(5);
    wxString anchor;

    if (path.Find('#', true) != wxNOT_FOUND)
    {
        anchor = path.AfterLast('#');
        path = path.BeforeLast('#');
    }

    for (unsigned int i = 0; i < filelistsize; ++i)
    {
        if (filelist[i].path == path)
        {
            wxLogDebug(_T("Found found : %s"), filelist[i].path.c_str());

            if (filelist[i].decompressed_data.size() == 0)
            {
                // decompress file data
                filelist[i].decompressed_data = decompress(filelist[i].compressed_data,
                                                           filelist[i].compressed_size,
                                                           filelist[i].uncompressed_size);
            }

            wxMemoryInputStream* ms = new wxMemoryInputStream(filelist[i].decompressed_data.data(),
                                                              filelist[i].decompressed_data.size());

            return new wxFSFile(ms,
                                location,
                                GetMimeTypeFromExt(path),
                                anchor,
                                wxDateTime::Now());
        }
    }

    wxLogDebug(_T("BuiltinHelp: could not serve file %s"), location.c_str());

    return NULL;
}

#include "help/en/html/back.gif.h"
#include "help/en/html/contents.gif.h"
#include "help/en/html/forward.gif.h"
#include "help/en/html/up.gif.h"
#include "help/en/html/cryptote.hhc.h"
#include "help/en/html/cryptote.hhk.h"
#include "help/en/html/cryptote.hhp.h"
#include "help/en/html/cryptote.hhp.cached.h"
#include "help/en/html/cryptote_contents.html.h"
#include "help/en/html/cryptote_introduction.html.h"
#include "help/en/html/cryptote_aboutencryption.html.h"
#include "help/en/html/cryptote_features.html.h"

#include "help/de/html/back.gif.h"
#include "help/de/html/contents.gif.h"
#include "help/de/html/forward.gif.h"
#include "help/de/html/up.gif.h"
#include "help/de/html/cryptote.hhc.h"
#include "help/de/html/cryptote.hhk.h"
#include "help/de/html/cryptote.hhp.h"
#include "help/de/html/cryptote.hhp.cached.h"
#include "help/de/html/cryptote_contents.html.h"
#include "help/de/html/cryptote_einfuehrung.html.h"
#include "help/de/html/cryptote_ueberverschluesselung.html.h"
#include "help/de/html/cryptote_funktionsumfang.html.h"

#define FILE(path, buffer)       { wxString(wxT(path)), buffer, sizeof(buffer), buffer ## _uncompressed, std::string() }

struct BuiltinHtmlHelpFSHandler::BuiltinFile
BuiltinHtmlHelpFSHandler::filelist[] =
{
    FILE("en/back.gif", help_en_back_gif),
    FILE("en/contents.gif", help_en_contents_gif),
    FILE("en/forward.gif", help_en_forward_gif),
    FILE("en/up.gif", help_en_up_gif),

    FILE("en/cryptote.hhc", help_en_cryptote_hhc),
    FILE("en/cryptote.hhk", help_en_cryptote_hhk),
    FILE("en/cryptote.hhp", help_en_cryptote_hhp),
    FILE("en/cryptote.hhp.cached", help_en_cryptote_hhp_cached),
    FILE("en/cryptote_contents.html", help_en_cryptote_contents_html),
    FILE("en/cryptote_introduction.html", help_en_cryptote_introduction_html),
    FILE("en/cryptote_aboutencryption.html", help_en_cryptote_aboutencryption_html),
    FILE("en/cryptote_features.html", help_en_cryptote_features_html),

    FILE("de/back.gif", help_de_back_gif),
    FILE("de/contents.gif", help_de_contents_gif),
    FILE("de/forward.gif", help_de_forward_gif),
    FILE("de/up.gif", help_de_up_gif),

    FILE("de/cryptote.hhc", help_de_cryptote_hhc),
    FILE("de/cryptote.hhk", help_de_cryptote_hhk),
    FILE("de/cryptote.hhp", help_de_cryptote_hhp),
    FILE("de/cryptote.hhp.cached", help_de_cryptote_hhp_cached),
    FILE("de/cryptote_contents.html", help_de_cryptote_contents_html),
    FILE("de/cryptote_einfuehrung.html", help_de_cryptote_einfuehrung_html),
    FILE("de/cryptote_ueberverschluesselung.html", help_de_cryptote_ueberverschluesselung_html),
    FILE("de/cryptote_funktionsumfang.html", help_de_cryptote_funktionsumfang_html)
};

unsigned int
BuiltinHtmlHelpFSHandler::filelistsize = sizeof(filelist) / sizeof(filelist[0]);

/******************************************************************************/
