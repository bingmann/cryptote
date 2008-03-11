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

    /// Application exit function
    virtual int		OnExit()
    {
	return 0;
    }
};

// This implements main(), WinMain() or whatever
IMPLEMENT_APP(App)
