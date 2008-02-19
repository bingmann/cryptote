// $Id$

#ifndef CEWMAIN_H
#define CEWMAIN_H

#include <wx/wx.h>

class CEWMain : public wxFrame
{
public:
    CEWMain(wxWindow* parent);

    // *** Event Handlers ***

    // Menu Events

    void	OnMenuFileNew(wxCommandEvent& event);
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
    void 	CreateMenuBar();

    wxStatusBar* statusbar;

    DECLARE_EVENT_TABLE()
};

#endif // CEWMAIN_H
