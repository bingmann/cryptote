/*******************************************************************************
 * src/pwgen/main.cpp
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

#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/config.h>

#include "wpassgen.h"
#include "enctain.h"

#include "common/myintl.h"
#include "locale/de.h"
#include "locale/wxstd/de.h"

static MyLocaleMemoryCatalog cryptote_catalogs[] =
{
    { _T("de"), NULL, locale_de_mo, sizeof(locale_de_mo), locale_de_mo_uncompressed },
    { NULL, NULL, NULL, 0, 0 }
};

static MyLocaleMemoryCatalog wxstd_catalogs[] =
{
    { _T("de"), NULL, locale_wxstd_de_mo, sizeof(locale_wxstd_de_mo), locale_wxstd_de_mo_uncompressed },
    { NULL, NULL, NULL, 0, 0 }
};

class App : public wxApp
{
private:
    /// Password generator main dialog
    class WPassGen* wmain;

    /// Locale object holding translations
    MyLocale* locale;

    /// Enctain Library Initializer
    Enctain::LibraryInitializer enctain_init;

public:
    /// This function is called during application start-up.
    virtual bool OnInit()
    {
        wxLog::SetActiveTarget(new wxLogStderr);

        SetAppName(_T("CryptoTE"));
        SetVendorName(_T("panthema.net"));

        // Setup locale to system default

        locale = new MyLocale(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);

        // Load and initialize the catalog
        if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
            !locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
        {
            // Could not load message catalog for system language, falling back
            // to English.

            delete locale;
            locale = new MyLocale(wxLANGUAGE_ENGLISH, wxLOCALE_CONV_ENCODING);

            if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs) ||
                !locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
            {
                wxLogError(_T("Could not load message catalog for system or English language."));
                return false;
            }
        }

        if (!wxApp::OnInit()) return false;

        wxImage::AddHandler(new wxPNGHandler());

        wxLog::SetActiveTarget(NULL);

        // Create main window frame
        wmain = new WPassGen(NULL, true);
        SetTopWindow(wmain);

        // Load config presets and settings from registry/user-configfile
        wxConfigBase* cfg = wxConfigBase::Get();
        wmain->LoadSettings(cfg);

        // Show Dialog
        wmain->Show();

        return true;
    }

    virtual void OnInitCmdLine(wxCmdLineParser& parser)
    {
        parser.AddSwitch(_T("h"), _T("help"),
                         _T("Display help for the command line parameters."),
                         wxCMD_LINE_OPTION_HELP);

        parser.AddOption(_T("L"), _T("lang"),
                         _T("Set language for messages. Example: de or de_DE."),
                         wxCMD_LINE_VAL_STRING, 0);
    }

    virtual bool OnCmdLineParsed(wxCmdLineParser& parser)
    {
        wxLog::SetActiveTarget(new wxLogStderr);

        // First thing to do: set up language and locale
        wxString langtext;
        if (parser.Found(_T("L"), &langtext))
        {
            const wxLanguageInfo* langinfo = wxLocale::FindLanguageInfo(langtext);

            if (!langinfo) {
                wxLogError(_("Invalid language identifier specified with --lang."));
                return false;
            }

            if (locale) delete locale;
            locale = new MyLocale;

            if (!locale->Init(langinfo->Language, wxLOCALE_CONV_ENCODING)) {
                wxLogError(_("This language is not supported by the program."));
                return false;
            }

            // Load and initialize the catalog
            if (!locale->AddCatalogFromMemory(_T("cryptote"), cryptote_catalogs))
            {
                wxLogError(_("This language is not supported by the program."));
                return false;
            }

            // Load and initialize the catalog
            if (!locale->AddCatalogFromMemory(_T("wxstd"), wxstd_catalogs))
            {
                wxLogError(_("This language is not supported by the program."));
                return false;
            }
        }

        wxLog::SetActiveTarget(NULL);
        return true;
    }

    /// Application exit function
    virtual int OnExit()
    {
        return 0;
    }
};

// This implements main(), WinMain() or whatever
IMPLEMENT_APP(App)

/******************************************************************************/
