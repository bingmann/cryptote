// $Id$

#include <wx/wx.h>

#include "wgenpass.h"

class App : public wxApp
{
private:
    /// Main window frame
    class WGeneratePassword*	wmain;

public:
    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	wxInitAllImageHandlers();

	// Create main window frame
	wmain = new WGeneratePassword(NULL);
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
