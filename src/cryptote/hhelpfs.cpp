// $Id$

#include "hhelpfs.h"

#include "common/tools.h"
#include <wx/log.h>

BuiltinHtmlHelpFSHandler::BuiltinHtmlHelpFSHandler()
{
}

BuiltinHtmlHelpFSHandler::~BuiltinHtmlHelpFSHandler()
{
}

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

    if (path.Find('#',true) != wxNOT_FOUND)
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

            wxMemoryInputStream* ms = new wxMemoryInputStream( filelist[i].decompressed_data.data(),
                                                               filelist[i].decompressed_data.size() );

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

#define FILE(path,buffer)       { wxString(wxT(path)), buffer, sizeof(buffer), buffer##_uncompressed, std::string() }

struct BuiltinHtmlHelpFSHandler::BuiltinFile
BuiltinHtmlHelpFSHandler::filelist[] =
{
    FILE("back.gif", help_en_back_gif),
    FILE("contents.gif", help_en_contents_gif),
    FILE("forward.gif", help_en_forward_gif),
    FILE("up.gif", help_en_up_gif),

    FILE("cryptote.hhc", help_en_cryptote_hhc),
    FILE("cryptote.hhk", help_en_cryptote_hhk),
    FILE("cryptote.hhp", help_en_cryptote_hhp),
    FILE("cryptote.hhp.cached", help_en_cryptote_hhp_cached),
    FILE("cryptote_contents.html", help_en_cryptote_contents_html),
    FILE("cryptote_introduction.html", help_en_cryptote_introduction_html),
    FILE("cryptote_aboutencryption.html", help_en_cryptote_aboutencryption_html),
    FILE("cryptote_features.html", help_en_cryptote_features_html),
};

unsigned int
BuiltinHtmlHelpFSHandler::filelistsize = sizeof(filelist) / sizeof(filelist[0]);
