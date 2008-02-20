// $Id$

#ifndef CEWMAIN_H
#define CEWMAIN_H

#include <wx/wx.h>

class CEWMain : public wxFrame
{
public:
    CEWMain(wxWindow* parent);

    // *** Identifiers ***

    enum {
	myID_EDITCTRL = wxID_HIGHEST + 1,
	myID_MENU_SELECTLINE
    };

    // *** Operations ***

    /// Update the title bar with the currently loaded text file name
    void	UpdateTitle();

    /// Update status bar to show given text.
    void	UpdateStatusBar(const wxString& str);

    /// Enable or Disable Save and SaveAs depending on if the buffer is modified.
    void	UpdateOnSavePoint();

    // *** Event Handlers ***

    // Menu Events

    void	OnMenuFileOpen(wxCommandEvent& event);
    void	OnMenuFileSave(wxCommandEvent& event);
    void	OnMenuFileSaveAs(wxCommandEvent& event);
    void	OnMenuFileRevert(wxCommandEvent& event);
    void	OnMenuFileClose(wxCommandEvent& event);

    void	OnMenuFileQuit(wxCommandEvent& event);

    void	OnMenuEditGeneric(wxCommandEvent& event);

    void	OnMenuHelpAbout(wxCommandEvent& event);

    void	OnScintillaUpdateUI(class wxStyledTextEvent& event);
    void	OnScintillaSavePointReached(class wxStyledTextEvent& event);
    void	OnScintillaSavePointLeft(class wxStyledTextEvent& event);

protected:
    // *** Menu and Status Bars of the main window ***

    wxMenuBar*	menubar;
    wxToolBar*	toolbar;

    void 	CreateMenuBar();

    wxStatusBar* statusbar;

    // *** Styled Text Edit control ***

    class CEWEdit* editctrl;

    DECLARE_EVENT_TABLE()
};

#endif // CEWMAIN_H
