// $Id$

#include <wx/wx.h>
#include <wx/cmdline.h>

#include "wcryptote.h"

class App : public wxApp
{
private:
    /// CryptoTE main dialog
    class WCryptoTE*	wmain;

    /// File path to load initially
    wxString		cmdlinefile;

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

	parser.AddParam(wxT("container-to-load"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars(wxT("-"));
    }

    bool	OnCmdLineParsed(wxCmdLineParser& parser)
    {
	if (parser.GetParamCount() > 0)
	    cmdlinefile = parser.GetParam(0);

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
