// $Id$

#include <wx/wx.h>

#include "cewmain.h"

class App : public wxApp
{
private:
    /// CryptoTE main dialog
    class CEWMain*	wmain;

public:
    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	wxInitAllImageHandlers();

	SetAppName(_("CryptoTE"));
	SetVendorName(_("idlebox.net"));

	// Create main window frame
	wmain = new CEWMain(NULL);
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
