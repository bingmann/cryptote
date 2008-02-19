// $Id$

#ifndef CEWMAIN_H
#define CEWMAIN_H

#include <wx/wx.h>

class CEWMain : public wxFrame
{
public:
    CEWMain(wxWindow* parent);

    // *** Operations ***

    /// Update the title bar with the currently loaded text file name
    void	UpdateTitle();

    /// Update status bar to show given text.
    void	UpdateStatusBar(const wxString& str);

    // *** Event Handlers ***

    // Menu Events

    void	OnMenuFileOpen(wxCommandEvent& event);
    void	OnMenuFileSave(wxCommandEvent& event);
    void	OnMenuFileSaveAs(wxCommandEvent& event);
    void	OnMenuFileRevert(wxCommandEvent& event);
    void	OnMenuFileClose(wxCommandEvent& event);

    void	OnMenuFileQuit(wxCommandEvent& event);

    void	OnMenuHelpAbout(wxCommandEvent& event);

protected:
    // *** Menu and Status Bars of the main window ***

    wxMenuBar*	menubar;
    wxToolBar*	toolbar;

    void 	CreateMenuBar();

    wxStatusBar* statusbar;

    // *** Styled Text Edit control ***

    class CEWEdit* edit;

    DECLARE_EVENT_TABLE()
};

#endif // CEWMAIN_H
