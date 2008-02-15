// $Id$

#include <wx/wx.h>

class App : public wxApp
{
public:
    /// This function is called during application start-up.
    virtual bool	OnInit()
    {
	wxInitAllImageHandlers();

	return false;
    }

    /// Application exit function
    virtual int		OnExit()
    {
	return 0;
    }
};

// This implements main(), WinMain() or whatever
IMPLEMENT_APP(App)
