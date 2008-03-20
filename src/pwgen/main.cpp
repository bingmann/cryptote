// $Id$

#include <wx/wx.h>
#include <wx/cmdline.h>

#include "wpassgen.h"
#include "common/myintl.h"
#include "locale/de.h"

static MyLocaleMemoryCatalogLanguage cryptote_cataloglangs[] =
{
    { _T("de"), NULL, locale_de_mo, sizeof(locale_de_mo), locale_de_mo_uncompressed },
    { NULL, NULL, NULL, 0, 0 }
};

static MyLocaleMemoryCatalog cryptote_catalog =
{
    _T("cryptote"), cryptote_cataloglangs
};

class App : public wxApp
{
private:
    /// Password generator main dialog
    class WPassGen*	wmain;

    /// Locale object holding translations
    MyLocale		locale;

public:
    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	if (!wxApp::OnInit()) return false;

	wxImage::AddHandler(new wxPNGHandler());

	SetAppName(_("CryptoTE"));
	SetVendorName(_("idlebox.net"));

	// Load and initialize the catalog
	if (!locale.AddCatalogFromMemory(cryptote_catalog))
	{
	    wxLogError(_T("Could not load message catalog for defined language."));
	    return false;
	}

#ifdef __LINUX__
	{   // Something from the wxWidgets example: load fileutils catalog for
	    // further messages
	    wxLogNull noLog;
	    locale.AddCatalog(_T("fileutils"));
	}
#endif

	// Create main window frame
	wmain = new WPassGen(NULL, true);
	SetTopWindow(wmain);
	wmain->Show();

	return true;
    }

    virtual void OnInitCmdLine(wxCmdLineParser& parser)
    {
        parser.AddSwitch(_T("h"), _T("help"),
			 _T("Display help for the command line parameters."),
			 wxCMD_LINE_OPTION_HELP);

        parser.AddOption(_T("l"), _T("lang"),
			 _T("Set language for messages. Example: de or de_DE."),
			 wxCMD_LINE_VAL_STRING, 0);
    }

    virtual bool OnCmdLineParsed(wxCmdLineParser& parser)
    {
	wxLog::SetActiveTarget(new wxLogStderr);

	wxString langtext;
	if ( parser.Found(_T("l"), &langtext) )
	{
	    const wxLanguageInfo* langinfo = wxLocale::FindLanguageInfo(langtext);

	    if (!langinfo) {
		wxLogError(_T("Invalid language identifier specified with --lang."));
		return false;
	    }

	    if (!locale.Init(langinfo->Language, wxLOCALE_CONV_ENCODING)) {
		wxLogError(_T("This language is not supported by the program."));
		return false;
	    }
	}
	else
	{
	    locale.Init(wxLANGUAGE_DEFAULT, wxLOCALE_CONV_ENCODING);
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
