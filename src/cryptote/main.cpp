// $Id$

#include <wx/wx.h>
#include <wx/cmdline.h>

#include "wcryptote.h"
#include "common/myintl.h"
#include "locale/de.h"

static MyLocaleMemoryCatalogLanguage cryptote_cataloglangs[] =
{
    { _T("de"), NULL, locale_de_mo, sizeof(locale_de_mo) },
    { NULL, NULL, NULL, 0 }
};

static MyLocaleMemoryCatalog cryptote_catalog =
{
    _T("cryptote"), cryptote_cataloglangs
};

class App : public wxApp
{
private:
    /// CryptoTE main dialog
    class WCryptoTE*	wmain;

    /// File path to load initially
    wxString		cmdlinefile;

    /// Locale object holding translations
    MyLocale		locale;

public:

    App()
	: wxApp(), wmain(NULL)
    {
    }

    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	// call parent-class for default behaviour and cmdline parsing
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
	wmain = new WCryptoTE(NULL);
	SetTopWindow(wmain);
	wmain->Show();

	if (!cmdlinefile.IsEmpty())
	{
	    wmain->ContainerOpen(cmdlinefile);
	}

	return true;
    }

    void	OnInitCmdLine(wxCmdLineParser& parser)
    {
        parser.AddSwitch(wxT("h"), wxT("help"),
			 _("Display help for the command line parameters."),
			 wxCMD_LINE_OPTION_HELP);

        parser.AddOption(_T("l"), _T("lang"),
			 _T("Set language for messages. Example: de or de_DE."),
			 wxCMD_LINE_VAL_STRING, 0);

	parser.AddParam(wxT("container-to-load"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars(wxT("-"));
    }

    bool	OnCmdLineParsed(wxCmdLineParser& parser)
    {
	wxLog::SetActiveTarget(new wxLogStderr);

	if (parser.GetParamCount() > 0)
	    cmdlinefile = parser.GetParam(0);

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

    virtual int OnExit()
    {
	return 0;
    }

    static inline bool IsUserEvent(const wxEvent& event)
    {
	// keyboard events
	if (event.GetEventType() == wxEVT_KEY_DOWN) return true;

	// mouse events
	if (event.GetEventType() == wxEVT_LEFT_DOWN) return true;
	if (event.GetEventType() == wxEVT_MIDDLE_DOWN) return true;
	if (event.GetEventType() == wxEVT_RIGHT_DOWN) return true;
	if (event.GetEventType() == wxEVT_LEFT_DCLICK) return true;
	if (event.GetEventType() == wxEVT_MIDDLE_DCLICK) return true;
	if (event.GetEventType() == wxEVT_RIGHT_DCLICK) return true;
	if (event.GetEventType() == wxEVT_MOUSEWHEEL) return true;

	// some extra events
	if (event.GetEventType() == wxEVT_MENU_OPEN) return true;
	if (event.GetEventType() == wxEVT_COMMAND_MENU_SELECTED) return true;

	return false;
    }

    /// This function received all event before they are processed by the
    /// target object. It monitors user events: keyboard and mouse actions and
    /// resets the idle-timer in the main window.
    virtual int FilterEvent(wxEvent& event)
    {
	if (wmain)
	{
	    if (IsUserEvent(event))
		wmain->ResetIdleTimer();
	}

	return -1;
    }
};

// This implements main(), WinMain() or whatever
IMPLEMENT_APP(App)
