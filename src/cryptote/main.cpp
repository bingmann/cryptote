// $Id$

#include <wx/wx.h>

#include "wcryptote.h"

class App : public wxApp
{
private:
    /// CryptoTE main dialog
    class WCryptoTE*	wmain;

public:
    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	wxImage::AddHandler(new wxPNGHandler());

	SetAppName(_("CryptoTE"));
	SetVendorName(_("idlebox.net"));

	// Create main window frame
	wmain = new WCryptoTE(NULL);
	SetTopWindow(wmain);
	wmain->Show();

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
